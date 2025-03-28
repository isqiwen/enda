/**
 * @file Takewhile.hpp
 *
 * Make an iterator that returns elements from the iterable as long as the predicate is true.
 */

#pragma once

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Fn, typename Iterator>
    class takewhile_iterator
    {
    public:
        takewhile_iterator(Fn fn, Iterator first, Iterator last) : _M_fn(fn), _M_it(first), _M_it_last(last) { next_selected(); }

        void next_selected()
        {
            for (; _M_it != _M_it_last && !_M_fn(*_M_it); ++_M_it)
                ;
        }

        decltype(auto) operator*() const { return *_M_it; }

        takewhile_iterator& operator++()
        {
            ++_M_it;
            next_selected();
            return *this;
        }

        bool operator==(const takewhile_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const takewhile_iterator& other) const { return !(*this == other); }

    private:
        Fn       _M_fn;
        Iterator _M_it;
        Iterator _M_it_last;
    };

    template<typename Fn, typename Iterator>
    auto takewhile(Fn predicate, Iterator first, Iterator last)
    {
        using it_t = takewhile_iterator<Fn, Iterator>;
        return range_view<it_t>(it_t(predicate, first, last), it_t(predicate, last, last));
    }

    template<typename Fn, typename Iterable>
    auto takewhile(Fn predicate, Iterable&& iterable)
    {
        return takewhile(predicate, iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
