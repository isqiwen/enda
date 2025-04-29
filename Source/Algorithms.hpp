/**
 * @file Algorithms.hpp
 *
 * @brief Provides various algorithms to be used with enda::Array objects.
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <utility>

#include "Concepts.hpp"
#include "Layout/ForEach.hpp"
#include "Traits.hpp"

namespace enda
{
    template<Array A, typename F, typename R>
    auto fold(F f, A const& a, R r)
    {
        // cast the initial value to the return type of f to avoid narrowing
        decltype(f(r, get_value_t<A> {})) r2 = r;
        enda::for_each(a.shape(), [&a, &r2, &f](auto&&... args) { r2 = f(r2, a(args...)); });
        return r2;
    }

    // The same as enda::fold, except that the initial value is a default constructed value type of the array.
    template<Array A, typename F>
    auto fold(F f, A const& a)
    {
        return fold(std::move(f), a, get_value_t<A> {});
    }

    template<Array A>
    bool any(A const& a)
    {
        static_assert(std::is_same_v<get_value_t<A>, bool>, "Error in enda::any: Value type of the array must be bool");
        return fold([](bool r, auto const& x) -> bool { return r or bool(x); }, a, false);
    }

    template<Array A>
    bool all(A const& a)
    {
        static_assert(std::is_same_v<get_value_t<A>, bool>, "Error in enda::all: Value type of the array must be bool");
        return fold([](bool r, auto const& x) -> bool { return r and bool(x); }, a, true);
    }

    template<Array A>
    auto max_element(A const& a)
    {
        return fold(
            [](auto const& x, auto const& y) {
                using std::max;
                return max(x, y);
            },
            a,
            get_first_element(a));
    }

    template<Array A>
    auto min_element(A const& a)
    {
        return fold(
            [](auto const& x, auto const& y) {
                using std::min;
                return min(x, y);
            },
            a,
            get_first_element(a));
    }

    template<ArrayOfRank<2> A>
    double frobenius_norm(A const& a)
    {
        return std::sqrt(fold(
            [](double r, auto const& x) -> double {
                auto ab = std::abs(x);
                return r + ab * ab;
            },
            a,
            double(0)));
    }

    template<Array A>
    auto sum(A const& a) requires(enda::is_scalar_v<get_value_t<A>>)
    {
        return fold(std::plus<> {}, a);
    }

    template<Array A>
    auto product(A const& a) requires(enda::is_scalar_v<get_value_t<A>>)
    {
        return fold(std::multiplies<> {}, a, get_value_t<A> {1});
    }

} // namespace enda
