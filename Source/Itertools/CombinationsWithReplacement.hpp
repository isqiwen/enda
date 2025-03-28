/**
 * @file CombinationsWithReplacement.hpp
 *
 *  Return successive r-length combinations_with_replacement of elements in the iterable
 *  allowing individual elements to have successive repeats.
 *
 *  combinations_with_replacement_with_replacement('ABC', 2) --> AA AB AC BB BC CC
 */

#pragma once

#include <algorithm>
#include <tuple>
#include <vector>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<std::size_t N, typename Iterator>
    class combinations_with_replacement_iterator;

    // base case: N = 1, behaves much like "regular" iterator.
    template<typename Iterator>
    class combinations_with_replacement_iterator<1, Iterator>
    {
    public:
        combinations_with_replacement_iterator(Iterator it, Iterator it_last) : _M_it(it), _M_it_last(it_last) {}

        void rewind(Iterator it) { _M_it = it; }

        bool exhausted() const { return _M_it == _M_it_last; }

        decltype(auto) operator*() const { return std::make_tuple(*_M_it); }

        combinations_with_replacement_iterator& operator++()
        {
            ++_M_it;
            return *this;
        }

        bool operator==(const combinations_with_replacement_iterator& other) { return _M_it == other._M_it; }

        bool operator!=(const combinations_with_replacement_iterator& other) { return !(*this == other); }

    private:
        Iterator _M_it;
        Iterator _M_it_last;
    };

    template<std::size_t N, typename Iterator>
    class combinations_with_replacement_iterator
    {
    public:
        combinations_with_replacement_iterator(Iterator it, Iterator it_last) : _M_it(it), _M_it_last(it_last), _M_sub_it(it, it_last) {}

        void rewind(Iterator it)
        {
            _M_it = it;
            _M_sub_it.rewind(_M_it);
        }

        bool exhausted() const { return _M_it == _M_it_last; }

        decltype(auto) operator*() const { return std::tuple_cat(std::make_tuple(*_M_it), *_M_sub_it); }

        combinations_with_replacement_iterator& operator++()
        {
            if ((++_M_sub_it).exhausted())
            {
                ++_M_it;
                _M_sub_it.rewind(_M_it);
            }
            return *this;
        }

        bool operator==(const combinations_with_replacement_iterator& other) { return _M_it == other._M_it && _M_sub_it == other._M_sub_it; }

        bool operator!=(const combinations_with_replacement_iterator& other) { return !(*this == other); }

    private:
        Iterator                                                _M_it;
        Iterator                                                _M_it_last;
        combinations_with_replacement_iterator<N - 1, Iterator> _M_sub_it;
    };

    template<std::size_t N, typename Iterator>
    auto combinations_with_replacement(Iterator first, Iterator last)
    {
        static_assert(N > 0);
        using comb_it_t    = combinations_with_replacement_iterator<N, Iterator>;
        std::size_t length = std::distance(first, last);
        if (length < N)
        {
            return range_view(comb_it_t(first, first), comb_it_t(first, first));
        }
        return range_view(comb_it_t(first, last), comb_it_t(last, last));
    }

} // namespace enda::itertools
