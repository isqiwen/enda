/**
 * @file LayoutTransform.hpp
 *
 * @brief Provides functions to transform the memory layout of an enda::basic_array or enda::basic_array_view.
 */

#pragma once

#include <array>
#include <concepts>
#include <type_traits>
#include <utility>

#include "Concepts.hpp"
#include "Declarations.hpp"
#include "GroupIndices.hpp"
#include "Layout/IdxMap.hpp"
#include "Layout/Permutation.hpp"
#include "Layout/Policies.hpp"
#include "Map.hpp"
#include "StdUtil/Array.hpp"
#include "Traits.hpp"

#ifndef NDEBUG
    #include <numeric>
#endif // NDEBUG

namespace enda
{

    /**
     * @addtogroup av_factories
     * @{
     */

    /**
     * @brief Transform the memory layout of an enda::basic_array or enda::basic_array_view.
     *
     * @details Depending on the input type `A`, the function does the following:
     * - If `A` is a regular rvalue array, return a new regular array with the given layout.
     * - Otherwise it returns a view with the given layout and
     *   - the same value type as `A` if `A` is non-const.
     *   - the const value type of `A` if `A` is const.
     *
     * The algebra of the transformed array/view is the same as the input array/view, except if its rank is different.
     * Then the algebra is set to 'A'.
     *
     * @tparam A enda::MemoryArray type.
     * @tparam NewLayoutType enda::idx_map type.
     * @param a Array/View to transform.
     * @param new_layout New memory layout.
     * @return If the input is an rvalue array, return a new array with the given layout. Otherwise, return a view with
     * the given layout.
     */
    template<MemoryArray A, typename NewLayoutType>
    auto map_layout_transform([[maybe_unused]] A&& a, [[maybe_unused]] NewLayoutType const& new_layout)
    {
        using A_t = std::remove_reference_t<A>;

        // layout policy of transformed array/view
        using layout_policy = typename detail::layout_to_policy<NewLayoutType>::type;

        // algebra of transformed array/view
        static constexpr auto algebra = (NewLayoutType::rank() == get_rank<A> ? get_algebra<A> : 'A');

        if constexpr (is_regular_v<A> and !std::is_reference_v<A>)
        {
            // return a transformed array if the given type is an rvalue array
            using array_t = basic_array<typename A_t::value_type, NewLayoutType::rank(), layout_policy, algebra, typename A_t::container_policy_t>;
            return array_t {new_layout, std::forward<A>(a).storage()};
        }
        else
        {
            // otherwise return a transformed view
            using value_t         = std::conditional_t<std::is_const_v<A_t>, const typename A_t::value_type, typename A_t::value_type>;
            using accessor_policy = typename get_view_t<A>::accessor_policy_t;
            using owning_policy   = typename get_view_t<A>::owning_policy_t;
            return basic_array_view<value_t, NewLayoutType::rank(), layout_policy, algebra, accessor_policy, owning_policy> {new_layout, a.storage()};
        }
    }

    /**
     * @brief Reshape an enda::basic_array or enda::basic_array_view.
     *
     * @details The input array/view is assumed to be contiguous and in C- or Fortran-order and the size of the reshaped
     * array/view must be the same as the input.
     *
     * It calls enda::map_layout_transform with the given shape.
     *
     * @tparam A enda::MemoryArray type.
     * @tparam Int Integral type.
     * @tparam R Number of dimensions of the new shape.
     * @param a Array/View to reshape.
     * @param new_shape Shape of the reshaped array/view.
     * @return An array/view with the new shape.
     */
    template<MemoryArray A, std::integral Int, auto R>
    auto reshape(A&& a, std::array<Int, R> const& new_shape)
    {
        // check size and contiguity of new shape
        EXPECTS_WITH_MESSAGE(a.size() == (std::accumulate(new_shape.cbegin(), new_shape.cend(), Int {1}, std::multiplies<> {})),
                             "Error in enda::reshape: New shape has an incorrect number of elements");
        EXPECTS_WITH_MESSAGE(a.indexmap().is_contiguous(), "Error in enda::reshape: Only contiguous arrays/views are supported");

        // restrict supported layouts (why?)
        using A_t = std::remove_cvref_t<A>;
        static_assert(A_t::is_stride_order_C() or A_t::is_stride_order_Fortran() or R == 1, "Error in enda::reshape: Only C or Fortran layouts are supported");

        // prepare new idx_map
        using layout_t = typename std::decay_t<A>::layout_policy_t::template mapping<R>;
        return map_layout_transform(std::forward<A>(a), layout_t {stdutil::make_std_array<long>(new_shape)});
    }

    /**
     * @brief Reshape an enda::basic_array or enda::basic_array_view.
     *
     * @details See enda::reshape(A &&, std::array< Int, R > const &).
     *
     * @tparam A enda::MemoryArray type.
     * @tparam Ints Integral types
     * @param a Array/View to reshape.
     * @param is Extents of the new shape.
     * @return An array/view with the new shape.
     */
    template<MemoryArray A, std::integral... Ints>
    auto reshape(A&& a, Ints... is)
    {
        return reshape(std::forward<A>(a), std::array<long, sizeof...(Ints)> {static_cast<long>(is)...});
    }

    /// @deprecated Use enda::reshape instead.
    template<MemoryArray A, std::integral Int, auto newRank>
    [[deprecated("Please use reshape(arr, shape) instead")]] auto reshaped_view(A&& a, std::array<Int, newRank> const& new_shape)
    {
        return reshape(std::forward<A>(a), new_shape);
    }

    /**
     * @brief Flatten an enda::basic_array or enda::basic_array_view to a 1-dimensional array/view by reshaping it.
     *
     * @details It calls enda::reshape with a 1-dimensional shape of the same size as the original array/view.
     *
     * @tparam A enda::MemoryArray type.
     * @param a Array/View to flatten.
     * @return An 1-dimensional array/view with the same size as the original array/view.
     */
    template<MemoryArray A>
    auto flatten(A&& a)
    {
        return reshape(std::forward<A>(a), std::array {a.size()});
    }

    /**
     * @brief Permute the indices/dimensions of an enda::basic_array or enda::basic_array_view.
     *
     * @details It first calls the enda::idx_map::transpose method of the enda::idx_map of the array/view to get the new
     * layout and then calls enda::map_layout_transform.
     *
     * @tparam Permutation Permutation encoded as an integer to apply to the indices.
     * @tparam A enda::MemoryArray type.
     * @param a Array/View to permute.
     * @return An array/view with permuted indices/dimensions.
     */
    template<uint64_t Permutation, MemoryArray A>
    auto permuted_indices_view(A&& a)
    {
        return map_layout_transform(std::forward<A>(a), a.indexmap().template transpose<Permutation>());
    }

    /**
     * @brief Transpose the memory layout of an enda::MemoryArray or an enda::expr_call.
     *
     * @details For enda::MemoryArray types, it calls enda::permuted_indices_view with the reverse identity permutation.
     * For enda::expr_call types, it calls enda::map with the transposed array.
     *
     * @tparam A enda::MemoryArray or enda::expr_call type.
     * @param a Array/View or expression call.
     * @return An array/view with transposed memory layout or a new enda::expr_call with the transposed array as the
     * argument.
     */
    template<typename A>
    auto transpose(A&& a) requires(MemoryArray<A> or is_instantiation_of_v<expr_call, A>)
    {
        if constexpr (MemoryArray<A>)
        {
            return permuted_indices_view<encode(enda::permutations::reverse_identity<get_rank<A>>())>(std::forward<A>(a));
        }
        else
        { // expr_call
            static_assert(std::tuple_size_v<decltype(a.a)> == 1, "Error in enda::transpose: Cannot transpose expr_call with more than one array argument");
            return map(a.f)(transpose(std::get<0>(std::forward<A>(a).a)));
        }
    }

    /**
     * @brief Transpose two indices/dimensions of an enda::basic_array or enda::basic_array_view.
     *
     * @details It calls enda::permuted_indices_view with the permutation that represents the transposition of the given
     * indices.
     *
     * @tparam I First index/dimension to transpose.
     * @tparam J Second index/dimension to transpose.
     * @tparam A enda::MemoryArray type.
     * @param a Array/View to transpose.
     * @return An array/view with transposed indices/dimensions.
     */
    template<int I, int J, MemoryArray A>
    auto transposed_view(A&& a) requires(is_regular_or_view_v<A>)
    {
        return permuted_indices_view<encode(permutations::transposition<get_rank<A>>(I, J))>(std::forward<A>(a));
    }

    // FIXME : use "magnetic" placeholder
    /**
     * @brief Create a new enda::basic_array or enda::basic_array_view by grouping indices together of a given array/view.
     *
     * @details The resulting array/view has the same number of dimensions as given index groups.
     *
     * It first calls enda::group_indices_layout to create a new enda::idx_map with the grouped indices and then calls
     * enda::map_layout_transform with the new layout.
     *
     * It can be used as follows:
     *
     * @code{.cpp}
     * auto arr = enda::rand<double>(3, 4, 5, 6);
     * auto view = enda::group_indices_view(arr, idx_group<0, 1>, idx_group<2, 3>);
     * @endcode
     *
     * In this examples a 4-dimensional array `arr` is transformed into a 2-dimensional view by grouping the first and
     * the last two indices together.
     *
     * @tparam A enda::MemoryArray type.
     * @tparam IdxGrps Groups of indices.
     * @param a Array/View to transform.
     * @return An array/view with grouped indices.
     */
    template<MemoryArray A, typename... IdxGrps>
    auto group_indices_view(A&& a, IdxGrps...)
    {
        return map_layout_transform(std::forward<A>(a), group_indices_layout(a.indexmap(), IdxGrps {}...));
    }

    namespace detail
    {

        // Append N fast dimensions to a given stride order.s
        template<int N, auto R>
        constexpr std::array<int, R + N> complete_stride_order_with_fast(std::array<int, R> const& order)
        {
            auto r = stdutil::make_initialized_array<R + N>(0);
            for (int i = 0; i < R; ++i)
                r[i] = order[i];
            for (int i = 0; i < N; ++i)
                r[R + i] = R + i;
            return r;
        }

    } // namespace detail

    /**
     * @brief Add `N` fast varying dimensions of size 1 to a given enda::basic_array or enda::basic_array_view.
     *
     * @tparam N Number of dimensions to add.
     * @tparam A enda::MemoryArray type.
     * @param a Array/View to transform.
     * @return An array/view with `N` dimensions of size 1 added to the original array/view.
     */
    template<int N, typename A>
    auto reinterpret_add_fast_dims_of_size_one(A&& a) requires(enda::is_regular_or_view_v<A>)
    {
        auto const& lay = a.indexmap();
        using lay_t     = std::decay_t<decltype(lay)>;

        static constexpr uint64_t new_stride_order_encoded   = encode(detail::complete_stride_order_with_fast<N>(lay_t::stride_order));
        static constexpr uint64_t new_static_extents_encoded = encode(stdutil::join(lay_t::static_extents, stdutil::make_initialized_array<N>(0)));
        using new_lay_t = idx_map<get_rank<A> + N, new_static_extents_encoded, new_stride_order_encoded, lay_t::layout_prop>;

        auto ones_n = stdutil::make_initialized_array<N>(1l);
        return map_layout_transform(std::forward<A>(a), new_lay_t {stdutil::join(lay.lengths(), ones_n), stdutil::join(lay.strides(), ones_n)});
    }

    /** @} */

} // namespace nda
