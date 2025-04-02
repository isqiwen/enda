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

    /**
     * @addtogroup av_algs
     * @{
     */

    // FIXME : CHECK ORDER of the LOOP !
    /**
     * @brief Perform a fold operation on the given enda::Array object.
     *
     * @details It calculates the following (where r is an initial value);
     *
     * @code{.cpp}
     * auto res = f(...f(f(f(r, a(0,...,0)), a(0,...,1)), a(0,...,2)), ...);
     * @endcode
     *
     * @note The array is always traversed in C-order.
     *
     * @tparam A enda::Array type.
     * @tparam F Callable type.
     * @tparam R Type of the initial value.
     * @param f Callable object taking two arguments compatible with the initial value and the array value type.
     * @param a enda::Array object.
     * @param r Initial value.
     * @return Result of the fold operation.
     */
    template<Array A, typename F, typename R>
    auto fold(F f, A const& a, R r)
    {
        // cast the initial value to the return type of f to avoid narrowing
        decltype(f(r, get_value_t<A> {})) r2 = r;
        enda::for_each(a.shape(), [&a, &r2, &f](auto&&... args) { r2 = f(r2, a(args...)); });
        return r2;
    }

    /// The same as enda::fold, except that the initial value is a default constructed value type of the array.
    template<Array A, typename F>
    auto fold(F f, A const& a)
    {
        return fold(std::move(f), a, get_value_t<A> {});
    }

    /**
     * @brief Does any of the elements of the array evaluate to true?
     *
     * @details The given enda::Array object can also be some lazy expression that evaluates to a boolean. For example:
     *
     * @code{.cpp}
     * auto A = enda::array<double, 2>::rand(2, 3);
     * auto greater05 = enda::map([](auto x) { return x > 0.5; })(A);
     * auto res = enda::any(greater05);
     * @endcode
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return True if at least one element of the array evaluates to true, false otherwise.
     */
    template<Array A>
    bool any(A const& a)
    {
        static_assert(std::is_same_v<get_value_t<A>, bool>, "Error in enda::any: Value type of the array must be bool");
        return fold([](bool r, auto const& x) -> bool { return r or bool(x); }, a, false);
    }

    /**
     * @brief Do all elements of the array evaluate to true?
     *
     * @details The given enda::Array object can also be some lazy expression that evaluates to a boolean. For example:
     *
     * @code{.cpp}
     * auto A = enda::array<double, 2>::rand(2, 3);
     * auto greater0 = enda::map([](auto x) { return x > 0.0; })(A);
     * auto res = enda::all(greater0);
     * @endcode
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return True if all elements of the array evaluate to true, false otherwise.
     */
    template<Array A>
    bool all(A const& a)
    {
        static_assert(std::is_same_v<get_value_t<A>, bool>, "Error in enda::all: Value type of the array must be bool");
        return fold([](bool r, auto const& x) -> bool { return r and bool(x); }, a, true);
    }

    /**
     * @brief Find the maximum element of an array.
     *
     * @details It uses enda::fold and `std::max`.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return Maximum element of the array.
     */
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

    /**
     * @brief Find the minimum element of an array.
     *
     * @details It uses enda::fold and `std::min`.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return Minimum element of the array.
     */
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

    /**
     * @ingroup av_math
     * @brief Calculate the Frobenius norm of a 2-dimensional array.
     *
     * @tparam A enda::ArrayOfRank<2> type.
     * @param a Array object.
     * @return Frobenius norm of the array/matrix.
     */
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

    /**
     * @brief Sum all the elements of an enda::Array object.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return Sum of all elements.
     */
    template<Array A>
    auto sum(A const& a) requires(enda::is_scalar_v<get_value_t<A>>)
    {
        return fold(std::plus<> {}, a);
    }

    /**
     * @brief Multiply all the elements of an enda::Array object.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return Product of all elements.
     */
    template<Array A>
    auto product(A const& a) requires(enda::is_scalar_v<get_value_t<A>>)
    {
        return fold(std::multiplies<> {}, a, get_value_t<A> {1});
    }

    /** @} */

} // namespace enda
