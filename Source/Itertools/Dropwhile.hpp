/**
 * @file Dropwhile.hpp
 *
 * Drop items from the iterable while predicate(item) is true.
 * Afterwards, return every element until the iterable is exhausted.
 */

#pragma once

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Iterator>
    using dropwhile_iterator = Iterator; /// dummy

    template<typename Fn, typename Iterator>
    auto dropwhile(Fn predicate, Iterator first, Iterator last)
    {
        for (; first != last && predicate(*first); ++first)
            ;
        return range_view<Iterator>(first, last);
    }

    template<typename Fn, typename Iterable>
    auto dropwhile(Fn predicate, Iterable&& iterable)
    {
        return dropwhile(predicate, iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
