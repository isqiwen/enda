#pragma once

#include <complex>
#include <cstdint>
#include <ranges>
#include <type_traits>
#include <utility>

namespace enda
{
    template<template<typename...> class TMPLT, typename T>
    struct is_instantiation_of : std::false_type
    {};

    template<template<typename...> class TMPLT, typename... U>
    struct is_instantiation_of<TMPLT, TMPLT<U...>> : std::true_type
    {};

    template<template<typename...> class TMPLT, typename T>
    inline constexpr bool is_instantiation_of_v = is_instantiation_of<TMPLT, std::remove_cvref_t<T>>::value;

    // Constexpr variable that is true if type `T` is contained in the parameter pack `Ts`.
    template<typename T, typename... Ts>
    inline constexpr bool is_any_of = (std::is_same_v<Ts, T> or ... or false);

    /// Constexpr variable that is always true regardless of the types in `Ts`.
    template<typename... Ts>
    inline constexpr bool always_true = true;

    /// Constexpr variable that is always false regardless of the types in `Ts` (used to trigger `static_assert`).
    template<typename... Ts>
    inline constexpr bool always_false = false;

    /// Constexpr variable that is true if type `T` is a std::complex type.
    template<typename T>
    inline constexpr bool is_complex_v = is_instantiation_of_v<std::complex, T>;

    /// Constexpr variable that is true if type `S` is a scalar type, i.e. arithmetic or complex.
    template<typename S>
    inline constexpr bool is_scalar_v = std::is_arithmetic_v<std::remove_cvref_t<S>> or is_complex_v<S>;

    /// Constexpr variable that specifies the rank of an nda::Array or of a contiguous 1-dimensional range.
    template<typename A>
    requires(std::ranges::contiguous_range<A> or requires { std::declval<A const>().shape(); }) constexpr int get_rank = []() {
        if constexpr (std::ranges::contiguous_range<A>)
            return 1;
        else
            return std::tuple_size_v<std::remove_cvref_t<decltype(std::declval<A const>().shape())>>;
    }();

    /**
     * @brief Get the first element of an array/view or simply return the scalar if a scalar is given.
     *
     * @tparam A Array/View/Scalar type.
     * @param a Input array/view/scalar.
     * @return If an array/view is given, return its first element. Otherwise, return the given scalar.
     */
    template<typename A>
    decltype(auto) get_first_element(A const& a)
    {
        if constexpr (is_scalar_v<A>)
        {
            return a;
        }
        else
        {
            return [&a]<auto... Is>(std::index_sequence<Is...>) -> decltype(auto) {
                return a((0 * Is)...); // repeat 0 sizeof...(Is) times
            }(std::make_index_sequence<get_rank<A>> {});
        }
    }

    /**
     * @brief Get the value type of an array/view or a scalar type.
     * @tparam A Array/View/Scalar type.
     */
    template<typename A>
    using get_value_t = std::decay_t<decltype(get_first_element(std::declval<A const>()))>;
} // namespace enda
