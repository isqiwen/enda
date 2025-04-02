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

    /**
     * @brief An empty base class that is not copyable or movable.
     *
     * The template parameter prevents multiple empty bases when inheriting multiple classes.
     */
    template<typename CRTP>
    struct immovable
    {
        immovable() = default;

        immovable(const immovable&) = delete;
        immovable(immovable&&)      = delete;

        auto operator=(const immovable&) -> immovable& = delete;
        auto operator=(immovable&&) -> immovable&      = delete;

        ~immovable() = default;
    };

    static_assert(std::is_empty_v<immovable<void>>);

    /** @} */

    /**
     * @addtogroup layout_utils
     * @{
     */

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

    /**
     * @brief Checks if two layout properties are compatible with each other.
     *
     * @param from Source nda::layout_prop_e.
     * @param to Target nda::layout_prop_e.
     * @return True if the source layout property is greater or equal to the target layout property,
     * otherwise false (see nda::layout_prop_e).
     */
    inline constexpr bool layout_property_compatible(layout_prop_e from, layout_prop_e to)
    {
        if (from == layout_prop_e::contiguous)
            return true;
        return ((to == layout_prop_e::none) or (to == from));
    }

    /**
     * @brief Bitwise OR operator for two layout properties.
     *
     * @param lhs Left hand side nda::layout_prop_e operand.
     * @param rhs Right hand side nda::layout_prop_e operand.
     * @return The bitwise OR of their numerical binary representation.
     */
    constexpr layout_prop_e operator|(layout_prop_e lhs, layout_prop_e rhs) { return layout_prop_e(uint64_t(lhs) | uint64_t(rhs)); }

    /**
     * @brief Bitwise AND operator for two layout properties.
     *
     * @param lhs Left hand side nda::layout_prop_e operand.
     * @param rhs Right hand side nda::layout_prop_e operand.
     * @return The bitwise AND of their numerical binary representation.
     */
    constexpr layout_prop_e operator&(layout_prop_e lhs, layout_prop_e rhs) { return layout_prop_e {uint64_t(lhs) & uint64_t(rhs)}; }

    /**
     * @brief Checks if a layout property has the `strided_1d` property.
     *
     * @param lp nda::layout_prop_e to check.
     * @return True if it has the `strided_1d` property, false otherwise.
     */
    inline constexpr bool has_strided_1d(layout_prop_e lp) { return uint64_t(lp) & uint64_t(layout_prop_e::strided_1d); }

    /**
     * @brief Checks if a layout property has the `smallest_stride_is_one` property.
     *
     * @param lp nda::layout_prop_e to check.
     * @return True if it has the `smallest_stride_is_one` property, false otherwise.
     */
    inline constexpr bool has_smallest_stride_is_one(layout_prop_e lp) { return uint64_t(lp) & uint64_t(layout_prop_e::smallest_stride_is_one); }

    /**
     * @brief Checks if a layout property has the `contiguous` property.
     *
     * @param lp nda::layout_prop_e to check.
     * @return True if it has the `contiguous` property, false otherwise.
     */
    inline constexpr bool has_contiguous(layout_prop_e lp) { return has_strided_1d(lp) and has_smallest_stride_is_one(lp); }

    // FIXME : I need a NONE for stride_order. For the scalars ...
    /**
     * @brief Stores information about the memory layout and the stride order of an array/view.
     *
     * @details The stride order of an N-dimensional array/view is a specific permutation of the integers from 0 to N-1,
     * i.e. `(i1, i2, ..., iN)`, which is encoded in a `uint64_t` (note that this limits the max. number of dimensions to
     * 16). It specifies the order in which the dimensions are traversed with `i1` being the slowest and `iN` the fastest.
     *
     * For example, a 3D array/view with C-order has a stride order of `(0, 1, 2)`, while the Fortran-order would be
     * `(2, 1, 0)`.
     */
    struct layout_info_t
    {
        /// Stride order of the array/view.
        uint64_t stride_order = 0;

        /// Memory layout properties of the array/view.
        layout_prop_e prop = layout_prop_e::none;
    };

    /**
     * @brief Bitwise AND operator for layout infos.
     *
     * @param lhs Left hand side nda::layout_info_t operand.
     * @param rhs Right hand side nda::layout_info_t operand.
     * @return A new nda::layout_info_t with the same stride order as the two arguments and their compatible
     * nda::layout_prop_e. If the stride orders are different, the stride order is set to -1 (undefined, no
     * permutation) and no memory layout guarantee can be made.
     */
    constexpr layout_info_t operator&(layout_info_t lhs, layout_info_t rhs)
    {
        if (lhs.stride_order == rhs.stride_order)
            return layout_info_t {lhs.stride_order, layout_prop_e(uint64_t(lhs.prop) & uint64_t(rhs.prop))};
        else
            return layout_info_t {uint64_t(-1), layout_prop_e::none};
    }

    /// Constexpr variable that specifies the nda::layout_info_t of type `A`.
    template<typename A>
    inline constexpr layout_info_t get_layout_info = layout_info_t {};

    // Specialization of nda::get_layout_info for cvref types.
    template<typename A>
    requires(!std::is_same_v<A, std::remove_cvref_t<A>>) inline constexpr layout_info_t get_layout_info<A> = get_layout_info<std::remove_cvref_t<A>>;

    /// Constexpr variable that is true if type `A` has the `contiguous` nda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_contiguous_layout = (has_contiguous(get_layout_info<A>.prop));

    /// Constexpr variable that is true if type `A` has the `strided_1d` nda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_layout_strided_1d = (has_strided_1d(get_layout_info<A>.prop));

    /// Constexpr variable that is true if type `A` has the `smallest_stride_is_one` nda::layout_prop_e guarantee.
    template<typename A>
    constexpr bool has_layout_smallest_stride_is_one = (has_smallest_stride_is_one(get_layout_info<A>.prop));

    /**
     * @brief A small wrapper around a single long integer to be used as a linear index.
     */
    struct _linear_index_t
    {
        /// Linear index.
        long value;
    };

    /** @} */
} // namespace enda
