/**
 * @file Tee.hpp
 *
 * Return n independent iterators from a single iterable.
 */

#pragma once

#include <tuple>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<unsigned N, typename T>
    struct duplicator;

    template<typename T>
    struct duplicator<0, T>
    {
        decltype(auto) get(T&& arg) const { return std::tuple<>(); }
    };

    template<unsigned N, typename T>
    struct duplicator
    {
        decltype(auto) get(T&& arg) const { return std::tuple_cat(std::make_tuple(arg), duplicator<N - 1, T>().get(arg)); }
    };

    template<unsigned N, typename Iterator>
    auto tee(Iterator first, Iterator last)
    {
        return duplicator<N, Iterator>().get(range_view<Iterator>(first, last));
    }

    template<unsigned N, typename Iterable>
    auto tee(Iterable&& iterable)
    {
        return duplicator<N, Iterable>().get(iterable);
    }

} // namespace enda::itertools
