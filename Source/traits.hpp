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

} // namespace enda
