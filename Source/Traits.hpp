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

    // Specialization of enda::is_instantiation_of for when it evaluates to true.
    template<template<typename...> class TMPLT, typename... U>
    struct is_instantiation_of<TMPLT, TMPLT<U...>> : std::true_type
    {};

    // Constexpr variable that is true if type `T` is an instantiation of `TMPLT` (see enda::is_instantiation_of).
    template<template<typename...> class TMPLT, typename T>
    inline constexpr bool is_instantiation_of_v = is_instantiation_of<TMPLT, std::remove_cvref_t<T>>::value;

    // Constexpr variable that is true if type `T` is contained in the parameter pack `Ts`.
    template<typename T, typename... Ts>
    static constexpr bool is_any_of = (std::is_same_v<Ts, T> or ... or false);

    // Constexpr variable that is always true regardless of the types in `Ts`.
    template<typename... Ts>
    static constexpr bool always_true = true;

    // Constexpr variable that is always false regardless of the types in `Ts` (used to trigger `static_assert`).
    template<typename... Ts>
    static constexpr bool always_false = false;

    // Constexpr variable that is true if type `T` is a std::complex type.
    template<typename T>
    inline constexpr bool is_complex_v = is_instantiation_of_v<std::complex, T>;

    // Constexpr variable that is true if type `S` is a scalar type, i.e. arithmetic or complex.
    template<typename S>
    inline constexpr bool is_scalar_v = std::is_arithmetic_v<std::remove_cvref_t<S>> or is_complex_v<S>;

    template<typename S>
    inline constexpr bool is_scalar_or_convertible_v = is_scalar_v<S> or std::is_constructible_v<std::complex<double>, S>;

    template<typename S, typename A>
    inline constexpr bool is_scalar_for_v = (is_scalar_v<typename A::value_type> ? is_scalar_or_convertible_v<S> : std::is_same_v<S, typename A::value_type>);

    // Constexpr variable that is true if type `T` is a std::complex type or a double type.
    template<typename T>
    inline constexpr bool is_double_or_complex_v = is_complex_v<T> or std::is_same_v<double, std::remove_cvref_t<T>>;

    template<typename A>
    inline constexpr char get_algebra = 'N';

    // Specialization of enda::get_algebra for cvref types.
    template<typename A>
    requires(!std::is_same_v<A, std::remove_cvref_t<A>>) inline constexpr char get_algebra<A> = get_algebra<std::remove_cvref_t<A>>;

    // Constexpr variable that specifies the rank of an enda::Array or of a contiguous 1-dimensional range.
    template<typename A>
    requires(std::ranges::contiguous_range<A> or requires { std::declval<A const>().shape(); }) constexpr int get_rank = []() {
        if constexpr (std::ranges::contiguous_range<A>)
            return 1;
        else
            return std::tuple_size_v<std::remove_cvref_t<decltype(std::declval<A const>().shape())>>;
    }();

    // Constexpr variable that is true if type `A` is a regular array, i.e. an enda::basic_array.
    template<typename A>
    inline constexpr bool is_regular_v = false;

    // Specialization of enda::is_regular_v for cvref types.
    template<typename A>
    requires(!std::is_same_v<A, std::remove_cvref_t<A>>) inline constexpr bool is_regular_v<A> = is_regular_v<std::remove_cvref_t<A>>;

    // Constexpr variable that is true if type `A` is a view, i.e. an enda::basic_array_view.
    template<typename A>
    inline constexpr bool is_view_v = false;

    // Specialization of enda::is_view_v for cvref types.
    template<typename A>
    requires(!std::is_same_v<A, std::remove_cvref_t<A>>) inline constexpr bool is_view_v<A> = is_view_v<std::remove_cvref_t<A>>;

    // Constexpr variable that is true if type `A` is either a regular array or a view.
    template<typename A>
    inline constexpr bool is_regular_or_view_v = is_regular_v<A> or is_view_v<A>;

    // Constexpr variable that is true if type `A` is a regular matrix or a view of a matrix.
    template<typename A>
    inline constexpr bool is_matrix_or_view_v = is_regular_or_view_v<A> and (get_algebra<A> == 'M') and (get_rank<A> == 2);

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

    template<typename A>
    using get_value_t = std::decay_t<decltype(get_first_element(std::declval<A const>()))>;

    // Constexpr variable that is true if all types in `As` have the same value type as `A0`.
    template<typename A0, typename... As>
    inline constexpr bool have_same_value_type_v = (std::is_same_v<get_value_t<A0>, get_value_t<As>> and ... and true);

    // Constexpr variable that is true if all types in `As` have the same rank as `A0`.
    template<typename A0, typename... As>
    inline constexpr bool have_same_rank_v = ((get_rank<A0> == get_rank<As>) and ... and true);

    /**
     * @brief Compile-time guarantees of the memory layout of an array/view.
     *
     * @details The possible values are:
     * - `none`: No guarantees.
     * - `strided_1d`: There is a constant overall stride in memory.
     * - `smallest_stride_is_one`: The stride in the fastest dimension is 1.
     * - `contiguous`: The array/view is contiguous in memory, i.e. it is `strided_1d` and `smallest_stride_is_one`.
     *
     * Furthermore, the set of values has a partial order:
     * - `contiguous` > `strided_1d` > `none`
     * - `contiguous` > `smallest_stride_is_one` > `none`
     */
    enum class layout_prop_e : uint64_t
    {
        none                   = 0x0,
        strided_1d             = 0x1,
        smallest_stride_is_one = 0x2,
        contiguous             = strided_1d | smallest_stride_is_one
    };

    inline constexpr bool layout_property_compatible(layout_prop_e from, layout_prop_e to)
    {
        if (from == layout_prop_e::contiguous)
            return true;
        return ((to == layout_prop_e::none) or (to == from));
    }

    constexpr layout_prop_e operator|(layout_prop_e lhs, layout_prop_e rhs) { return layout_prop_e(uint64_t(lhs) | uint64_t(rhs)); }

    constexpr layout_prop_e operator&(layout_prop_e lhs, layout_prop_e rhs) { return layout_prop_e {uint64_t(lhs) & uint64_t(rhs)}; }

    inline constexpr bool has_strided_1d(layout_prop_e lp) { return uint64_t(lp) & uint64_t(layout_prop_e::strided_1d); }

    inline constexpr bool has_smallest_stride_is_one(layout_prop_e lp) { return uint64_t(lp) & uint64_t(layout_prop_e::smallest_stride_is_one); }

    inline constexpr bool has_contiguous(layout_prop_e lp) { return has_strided_1d(lp) and has_smallest_stride_is_one(lp); }

    struct layout_info_t
    {
        // Stride order of the array/view.
        uint64_t stride_order = 0;

        // Memory layout properties of the array/view.
        layout_prop_e prop = layout_prop_e::none;
    };

    constexpr layout_info_t operator&(layout_info_t lhs, layout_info_t rhs)
    {
        if (lhs.stride_order == rhs.stride_order)
            return layout_info_t {lhs.stride_order, layout_prop_e(uint64_t(lhs.prop) & uint64_t(rhs.prop))};
        else
            return layout_info_t {uint64_t(-1), layout_prop_e::none};
    }

    // Constexpr variable that specifies the enda::layout_info_t of type `A`.
    template<typename A>
    inline constexpr layout_info_t get_layout_info = layout_info_t {};

    // Specialization of enda::get_layout_info for cvref types.
    template<typename A>
    requires(!std::is_same_v<A, std::remove_cvref_t<A>>) inline constexpr layout_info_t get_layout_info<A> = get_layout_info<std::remove_cvref_t<A>>;

    // Constexpr variable that is true if type `A` has the `contiguous` enda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_contiguous_layout = (has_contiguous(get_layout_info<A>.prop));

    // Constexpr variable that is true if type `A` has the `strided_1d` enda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_layout_strided_1d = (has_strided_1d(get_layout_info<A>.prop));

    // Constexpr variable that is true if type `A` has the `smallest_stride_is_one` enda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_layout_smallest_stride_is_one = (has_smallest_stride_is_one(get_layout_info<A>.prop));

    // A small wrapper around a single long integer to be used as a linear index.
    struct _linear_index_t
    {
        // Linear index.
        long value;
    };
} // namespace enda
