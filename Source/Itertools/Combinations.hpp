/**
 * @file Combinations.hpp
 *
 *  Return successive r-length combinations of elements in the iterable.
 *
 *  combinations(range(4), 3) --> (0,1,2), (0,1,3), (0,2,3), (1,2,3)
 */

#pragma once

#include <algorithm>
#include <tuple>
#include <vector>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<std::size_t N, typename Iterator>
    class combinations_iterator;

    // base case: N = 1, behaves much like "regular" iterator.
    template<typename Iterator>
    class combinations_iterator<1, Iterator>
    {
    public:
        combinations_iterator(Iterator it, Iterator it_last) : _M_it(it), _M_it_last(it_last) {}

        void rewind(Iterator it) { _M_it = it; }

        bool exhausted() const { return _M_it == _M_it_last; }

        decltype(auto) operator*() const { return std::make_tuple(*_M_it); }

        combinations_iterator& operator++()
        {
            ++_M_it;
            return *this;
        }

        bool operator==(const combinations_iterator& other) { return _M_it == other._M_it; }

        bool operator!=(const combinations_iterator& other) { return !(*this == other); }

    private:
        Iterator _M_it;
        Iterator _M_it_last;
    };

    template<std::size_t N, typename Iterator>
    class combinations_iterator
    {
    public:
        combinations_iterator(Iterator it, Iterator it_last) : _M_it(it), _M_it_last(it_last), _M_sub_it(std::next(it), std::next(it_last)) {}

        void rewind(Iterator it)
        {
            _M_it = it;
            _M_sub_it.rewind(std::next(_M_it));
        }

        bool exhausted() const { return _M_it == _M_it_last; }

        decltype(auto) operator*() const { return std::tuple_cat(std::make_tuple(*_M_it), *_M_sub_it); }

        combinations_iterator& operator++()
        {
            if ((++_M_sub_it).exhausted())
            {
                ++_M_it;
                _M_sub_it.rewind(std::next(_M_it));
            }
            return *this;
        }

        bool operator==(const combinations_iterator& other) { return _M_it == other._M_it && _M_sub_it == other._M_sub_it; }

        bool operator!=(const combinations_iterator& other) { return !(*this == other); }

    private:
        Iterator                               _M_it;
        Iterator                               _M_it_last;
        combinations_iterator<N - 1, Iterator> _M_sub_it;
    };

    template<std::size_t N, typename Iterator>
    auto combinations(Iterator first, Iterator last)
    {
        static_assert(N > 0);
        using comb_it_t    = combinations_iterator<N, Iterator>;
        std::size_t length = std::distance(first, last);
        if (length < N)
        {
            return range_view(comb_it_t(first, first), comb_it_t(first, first));
        }
        Iterator it = std::next(first, 1 + length - N);
        return range_view(comb_it_t(first, it), comb_it_t(it, it));
    }

    template<std::size_t N, typename Iterable>
    auto combinations(Iterable&& iterable)
    {
        return combinations<N>(iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
