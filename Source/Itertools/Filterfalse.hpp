/**
 * @file Filterfalse.hpp
 *
 * Return those items of sequence for which function(item) is false.
 * If function is None, return the items that are false.
 */

#pragma once

#include "Itertools/Filter.hpp"

namespace enda::itertools
{
    template<typename Fn, typename Iterator>
    auto filterfalse(Fn predicate, Iterator first, Iterator last)
    {
        using value_type       = decltype(*first);
        auto reverse_predicate = [&predicate](const value_type& arg) { return predicate(arg) ? false : true; };
        using it_t             = filter_iterator<decltype(reverse_predicate), Iterator>;
        return range_view<it_t>(it_t(reverse_predicate, first, last), it_t(reverse_predicate, last, last));
    }

    template<typename Fn, typename Iterable>
    auto filterfalse(Fn predicate, Iterable&& iterable)
    {
        return filterfalse(predicate, iterable.begin(), iterable.end());
    }

    template<typename Iterator>
    auto filterfalse(Iterator first, Iterator last)
    {
        return filterfalse(boolean_transformer<decltype(*first)>(), first, last);
    }

    template<typename Iterable>
    auto filterfalse(Iterable&& iterable)
    {
        return filterfalse(iterable.begin(), iterable.end());
    }
} // namespace enda::itertools
