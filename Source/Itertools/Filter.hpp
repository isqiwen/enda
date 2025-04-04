/**
 * @file Filter.hpp
 *
 * Return an iterator yielding those items of iterable for which function(item)
 * is true. If function is None, return the items that are true.
 */

#pragma once

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Fn, typename Iterator>
    class filter_iterator
    {
    public:
        filter_iterator(Fn fn, Iterator it, Iterator last) : _M_predicate(fn), _M_it(it), _M_last(last) { next_selected(); }

        void next_selected()
        {
            for (; _M_it != _M_last && !_M_predicate(*_M_it); ++_M_it)
                ;
        }

        decltype(auto) operator*() const { return *_M_it; }

        filter_iterator& operator++()
        {
            ++_M_it;
            next_selected();
            return *this;
        }

        bool operator==(const filter_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const filter_iterator& other) const { return !(*this == other); }

    private:
        Fn       _M_predicate;
        Iterator _M_it;
        Iterator _M_last;
    };

    template<typename T>
    struct boolean_transformer
    {
        bool operator()(const T& arg) const { return arg ? true : false; }
    };

    template<typename Fn, typename Iterator>
    auto filter(Fn predicate, Iterator first, Iterator last)
    {
        using it_t = filter_iterator<Fn, Iterator>;
        return range_view<it_t>(it_t(predicate, first, last), it_t(predicate, last, last));
    }

    template<typename Fn, typename Iterable>
    auto filter(Fn predicate, Iterable&& iterable)
    {
        return filter(predicate, iterable.begin(), iterable.end());
    }

    template<typename Iterator>
    auto filter(Iterator first, Iterator last)
    {
        return filter(boolean_transformer<decltype(*first)>(), first, last);
    }

    template<typename Iterable>
    auto filter(Iterable&& iterable)
    {
        return filter(iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
