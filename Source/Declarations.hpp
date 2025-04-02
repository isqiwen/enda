/**
 * @file Declarations.hpp
 *
 * @brief Provides various convenient aliases and helper functions for enda::basic_array and enda::basic_array_view.
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

#include "Accessors.hpp"
#include "Concepts.hpp"
#include "Layout/IdxMap.hpp"
#include "Layout/Permutation.hpp"
#include "Layout/Policies.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/Policies.hpp"
#include "Traits.hpp"

namespace enda
{
    /// @cond
    // Forward declarations.
    template<int Rank>
    using shape_t = std::array<long, Rank>;

    template<int Rank, uint64_t StaticExtents, uint64_t StrideOrder, layout_prop_e LayoutProp>
    class idx_map;

    template<typename ValueType, int Rank, typename Layout, char Algebra, typename ContainerPolicy>
    class basic_array;

    template<typename ValueType,
             int Rank,
             typename Layout,
             char Algebra            = 'A',
             typename AccessorPolicy = enda::default_accessor, //, enda::no_alias_accessor, //,
             typename OwningPolicy   = enda::borrowed<>>
    class basic_array_view;

    template<char OP, Array A>
    struct expr_unary;

    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    struct expr;
    /// @endcond

    auto     _ = range::all;
    ellipsis ___;

    /**
     * @addtogroup av_types
     * @{
     */

    /**
     * @brief Alias template of an enda::basic_array with an 'A' algebra.
     *
     * @tparam ValueType Value type of the array.
     * @tparam Rank Rank of the array.
     * @tparam Layout Layout policy of the array.
     * @tparam ContainerPolicy Memory policy of the array.
     */
    template<typename ValueType, int Rank, typename Layout = C_layout, typename ContainerPolicy = heap<>>
    using array = basic_array<ValueType, Rank, Layout, 'A', ContainerPolicy>;

    /**
     * @brief Alias template of an enda::basic_array_view with an 'A' algebra, enda::default_accessor and enda::borrowed
     * owning policy.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_stride_layout>
    using array_view = basic_array_view<ValueType, Rank, Layout, 'A', default_accessor, borrowed<>>;

    /**
     * @brief Same as enda::array_view except for const value types.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_stride_layout>
    using array_const_view = basic_array_view<ValueType const, Rank, Layout, 'A', default_accessor, borrowed<>>;

    /**
     * @brief Same as enda::array_view except for contiguous memory layouts.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_layout>
    requires(has_contiguous(Layout::template mapping<Rank>::layout_prop)) using array_contiguous_view =
        basic_array_view<ValueType, Rank, Layout, 'A', default_accessor, borrowed<>>;

    /**
     * @brief Same as enda::array_const_view except for contiguous memory layouts.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_layout>
    requires(has_contiguous(Layout::template mapping<Rank>::layout_prop)) using array_contiguous_const_view =
        basic_array_view<ValueType const, Rank, Layout, 'A', default_accessor, borrowed<>>;

    /**
     * @brief Alias template of an enda::basic_array with rank 2 and an 'M' algebra.
     *
     * @tparam ValueType Value type of the matrix.
     * @tparam Layout Layout policy of the matrix.
     * @tparam ContainerPolicy Memory policy of the matrix.
     */
    template<typename ValueType, typename Layout = C_layout, typename ContainerPolicy = heap<>>
    using matrix = basic_array<ValueType, 2, Layout, 'M', ContainerPolicy>;

    /**
     * @brief Alias template of an enda::basic_array_view with rank 2, an 'M' algebra, enda::default_accessor and
     * enda::borrowed owning policy.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using matrix_view = basic_array_view<ValueType, 2, Layout, 'M', default_accessor, borrowed<>>;

    /**
     * @brief Same as enda::matrix_view except for const value types.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using matrix_const_view = basic_array_view<ValueType const, 2, Layout, 'M', default_accessor, borrowed<>>;

    /**
     * @brief Alias template of an enda::basic_array with rank 1 and a 'V' algebra.
     *
     * @tparam ValueType Value type of the vector.
     * @tparam ContainerPolicy Memory policy of the vector.
     */
    template<typename ValueType, typename ContainerPolicy = heap<>>
    using vector = basic_array<ValueType, 1, C_layout, 'V', ContainerPolicy>;

    /**
     * @brief Alias template of an enda::basic_array_view with rank 1, a 'V' algebra, enda::default_accessor and
     * enda::borrowed owning policy.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using vector_view = basic_array_view<ValueType, 1, Layout, 'V', default_accessor, borrowed<>>;

    /**
     * @brief Same as enda::vector_view except for const value types.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using vector_const_view = basic_array_view<ValueType const, 1, Layout, 'V', default_accessor, borrowed<>>;

    /**
     * @ingroup av_utils
     * @brief Encode the given shape into a single integer using the enda::encode function.
     *
     * @tparam Is Integer sequence describing the shape.
     * @param i0 Size of the first dimension.
     * @param is Sizes of the other dimensions.
     * @return `uint64_t` encoded shape.
     */
    template<typename... Is>
    constexpr uint64_t static_extents(int i0, Is... is)
    {
        if (i0 > 15)
            throw std::runtime_error("Error in enda::static_extents: Only 16 dimensions are supported!");
        return encode(std::array<int, sizeof...(Is) + 1> {i0, static_cast<int>(is)...});
    }

    /**
     * @brief Alias template of an enda::basic_array with static extents, contiguous C layout, 'A' algebra and enda::stack
     * container policy.
     *
     * @tparam ValueType Value type of the array.
     * @tparam N0 Size of the first dimension.
     * @tparam Ns Sizes of the other dimensions.
     */
    template<typename ValueType, int N0, int... Ns>
    using stack_array =
        enda::basic_array<ValueType,
                          1 + sizeof...(Ns),
                          enda::basic_layout<enda::static_extents(N0, Ns...), enda::C_stride_order<1 + sizeof...(Ns)>, enda::layout_prop_e::contiguous>,
                          'A',
                          enda::stack<N0*(Ns*... * 1)>>;

    /**
     * @brief Alias template of an enda::basic_array with rank 2, static extents, contiguous C layout, 'M' algebra and
     * enda::stack memory policy.
     *
     * @tparam ValueType Value type of the matrix.
     * @tparam N Number of rows.
     * @tparam M Number of columns.
     */
    template<typename ValueType, int N, int M>
    using stack_matrix = enda::basic_array<ValueType,
                                           2,
                                           enda::basic_layout<enda::static_extents(N, M), enda::C_stride_order<2>, enda::layout_prop_e::contiguous>,
                                           'M',
                                           enda::stack<static_cast<size_t>(N* M)>>;

    /**
     * @brief Alias template of an enda::basic_array with rank 1, static extents, contiguous C layout, 'V' algebra and
     * enda::stack memory policy.
     *
     * @tparam ValueType Value type of the vector.
     * @tparam N Size of the vector.
     */
    template<typename ValueType, int N>
    using stack_vector = enda::
        basic_array<ValueType, 1, enda::basic_layout<enda::static_extents(N), enda::C_stride_order<1>, enda::layout_prop_e::contiguous>, 'V', enda::stack<N>>;

    /**
     * @brief Similar to enda::array except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the array.
     * @tparam Rank Rank of the array.
     * @tparam Layout Layout policy of the array.
     */
    template<typename ValueType, int Rank, typename Layout = C_layout>
    using cuarray = basic_array<ValueType, Rank, Layout, 'A', heap<mem::Device>>;

    /**
     * @brief Similar to enda::array_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_stride_layout>
    using cuarray_view = basic_array_view<ValueType, Rank, Layout, 'A', default_accessor, borrowed<mem::Device>>;

    /**
     * @brief Similar to enda::array_const_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Rank Rank of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, int Rank, typename Layout = C_stride_layout>
    using cuarray_const_view = basic_array_view<ValueType const, Rank, Layout, 'A', default_accessor, borrowed<mem::Device>>;

    /**
     * @brief Similar to enda::matrix except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the matrix.
     * @tparam Layout Layout policy of the matrix.
     * @tparam ContainerPolicy Memory policy of the matrix.
     */
    template<typename ValueType, typename Layout = C_layout, typename ContainerPolicy = heap<mem::Device>>
    using cumatrix = basic_array<ValueType, 2, Layout, 'M', ContainerPolicy>;

    /**
     * @brief Similar to enda::matrix_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using cumatrix_view = basic_array_view<ValueType, 2, Layout, 'M', default_accessor, borrowed<mem::Device>>;

    /**
     * @brief Similar to enda::matrix_const_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using cumatrix_const_view = basic_array_view<ValueType const, 2, Layout, 'M', default_accessor, borrowed<mem::Device>>;

    /**
     * @brief Similar to enda::vector except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the vector.
     */
    template<typename ValueType>
    using cuvector = basic_array<ValueType, 1, C_layout, 'V', heap<mem::Device>>;

    /**
     * @brief Similar to enda::vector_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using cuvector_view = basic_array_view<ValueType, 1, Layout, 'V', default_accessor, borrowed<mem::Device>>;

    /**
     * @brief Similar to enda::vector_const_view except the memory is stored on the device.
     *
     * @tparam ValueType Value type of the view.
     * @tparam Layout Layout policy of the view.
     */
    template<typename ValueType, typename Layout = C_stride_layout>
    using cuvector_const_view = basic_array_view<ValueType const, 1, Layout, 'V', default_accessor, borrowed<mem::Device>>;

    /** @} */

    /**
     * @addtogroup av_utils
     * @{
     */

    /// Specialization of enda::is_regular_v for enda::basic_array.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename ContainerPolicy>
    inline constexpr bool is_regular_v<basic_array<ValueType, Rank, Layout, Algebra, ContainerPolicy>> = true;

    /// Specialization of enda::is_view_v for enda::basic_array_view.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename AccessorPolicy, typename OwningPolicy>
    inline constexpr bool is_view_v<basic_array_view<ValueType, Rank, Layout, Algebra, AccessorPolicy, OwningPolicy>> = true;

    /// Specialization of enda::get_algebra for enda::basic_array types.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename ContainerPolicy>
    inline constexpr char get_algebra<basic_array<ValueType, Rank, Layout, Algebra, ContainerPolicy>> = Algebra;

    /// Specialization of enda::get_algebra for enda::basic_array_view types.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename AccessorPolicy, typename OwningPolicy>
    inline constexpr char get_algebra<basic_array_view<ValueType, Rank, Layout, Algebra, AccessorPolicy, OwningPolicy>> = Algebra;

    /// Specialization of enda::get_layout_info for enda::basic_array types.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename ContainerPolicy>
    inline constexpr layout_info_t get_layout_info<basic_array<ValueType, Rank, Layout, Algebra, ContainerPolicy>> =
        basic_array<ValueType, Rank, Layout, Algebra, ContainerPolicy>::layout_t::layout_info;

    /// Specialization of enda::get_layout_info for enda::basic_array_view types.
    template<typename ValueType, int Rank, typename Layout, char Algebra, typename AccessorPolicy, typename OwningPolicy>
    inline constexpr layout_info_t get_layout_info<basic_array_view<ValueType, Rank, Layout, Algebra, AccessorPolicy, OwningPolicy>> =
        basic_array_view<ValueType, Rank, Layout, Algebra, AccessorPolicy, OwningPolicy>::layout_t::layout_info;

    /**
     * @brief Get the type of the enda::basic_array_view that would be obtained by constructing a view from a given type.
     * @tparam T Type to construct a view from.
     */
    template<typename T, typename T2 = std::remove_reference_t<T> /* Keep this: Fix for gcc11 bug */>
    using get_view_t = std::remove_reference_t<decltype(basic_array_view {std::declval<T>()})>;

    /**
     * @brief Get the type of the enda::basic_array that would be obtained by constructing an array from a given type.
     * @tparam T Type to construct an array from.
     */
    template<typename T, typename T2 = std::remove_reference_t<T> /* Keep this: Fix for gcc11 bug */>
    using get_regular_t = decltype(basic_array {std::declval<T>()});

    /**
     * @brief Get the type of the enda::basic_array that would be obtained by constructing an array on host memory from a
     * given type.
     *
     * @tparam T Type to construct an array from.
     */
    template<typename T, typename RT = get_regular_t<T>>
    using get_regular_host_t = std::conditional_t<mem::on_host<RT>,
                                                  RT,
                                                  basic_array<get_value_t<RT>,
                                                              get_rank<RT>,
                                                              get_contiguous_layout_policy<get_rank<RT>, get_layout_info<RT>.stride_order>,
                                                              get_algebra<RT>,
                                                              heap<mem::Host>>>;

    /**
     * @brief Get the type of the enda::basic_array that would be obtained by constructing an array on device memory from
     * a given type.
     *
     * @tparam T Type to construct an array from.
     */
    template<typename T, typename RT = get_regular_t<T>>
    using get_regular_device_t = std::conditional_t<mem::on_device<RT>,
                                                    RT,
                                                    basic_array<get_value_t<RT>,
                                                                get_rank<RT>,
                                                                get_contiguous_layout_policy<get_rank<RT>, get_layout_info<RT>.stride_order>,
                                                                get_algebra<RT>,
                                                                heap<mem::Device>>>;

    /**
     * @brief Get the type of the enda::basic_array that would be obtained by constructing an array on unified memory from
     * a given type.
     *
     * @tparam T Type to construct an array from.
     */
    template<typename T, typename RT = get_regular_t<T>>
    using get_regular_unified_t = std::conditional_t<mem::on_unified<RT>,
                                                     RT,
                                                     basic_array<get_value_t<RT>,
                                                                 get_rank<RT>,
                                                                 get_contiguous_layout_policy<get_rank<RT>, get_layout_info<RT>.stride_order>,
                                                                 get_algebra<RT>,
                                                                 heap<mem::Unified>>>;

    /// Specialization of enda::get_algebra for enda::expr_unary types.
    template<char OP, Array A>
    inline constexpr char get_algebra<expr_unary<OP, A>> = get_algebra<A>;

    /// Specialization of enda::get_layout_info for enda::expr_unary types.
    template<char OP, Array A>
    inline constexpr layout_info_t get_layout_info<expr_unary<OP, A>> = get_layout_info<A>;

    /// Specialization of enda::get_algebra for enda::expr types.
    template<char OP, typename L, typename R>
    inline constexpr char get_algebra<expr<OP, L, R>> = expr<OP, L, R>::algebra;

    /// Specialization of enda::get_layout_info for enda::expr types.
    template<char OP, typename L, typename R>
    inline constexpr layout_info_t get_layout_info<expr<OP, L, R>> = expr<OP, L, R>::compute_layout_info();

    /** @} */

} // namespace enda
