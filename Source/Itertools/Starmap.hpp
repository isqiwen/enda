/**
 * @file Starmap.hpp
 *
 * Make an iterator that computes the function using arguments obtained from the iterable.
 */

#pragma once

#include <tuple>
#include <utility>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Fn, typename Iterator>
    class starmap_iterator
    {
    public:
        starmap_iterator(Fn fn, Iterator it) : _M_fn(fn), _M_it(it) {}

        decltype(auto) operator*() const { return std::apply(_M_fn, *_M_it); }

        starmap_iterator& operator++()
        {
            ++_M_it;
            return *this;
        }

        bool operator==(const starmap_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const starmap_iterator& other) const { return !(*this == other); }

    private:
        Fn       _M_fn;
        Iterator _M_it;
    };

    template<typename Fn, typename Iterator>
    auto starmap(Fn fn, Iterator first, Iterator last)
    {
        using it_t = starmap_iterator<Fn, Iterator>;
        return range_view<it_t>(it_t(fn, first), it_t(fn, last));
    }

    template<typename Fn, typename Iterable>
    auto starmap(Fn fn, Iterable&& iterable)
    {
        return starmap(fn, iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
