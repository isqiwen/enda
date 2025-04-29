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

namespace enda
{
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

    template<MemoryArray A, std::integral... Ints>
    auto reshape(A&& a, Ints... is)
    {
        return reshape(std::forward<A>(a), std::array<long, sizeof...(Ints)> {static_cast<long>(is)...});
    }

    template<MemoryArray A>
    auto flatten(A&& a)
    {
        return reshape(std::forward<A>(a), std::array {a.size()});
    }

    template<uint64_t Permutation, MemoryArray A>
    auto permuted_indices_view(A&& a)
    {
        return map_layout_transform(std::forward<A>(a), a.indexmap().template transpose<Permutation>());
    }

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

    template<int I, int J, MemoryArray A>
    auto transposed_view(A&& a) requires(is_regular_or_view_v<A>)
    {
        return permuted_indices_view<encode(permutations::transposition<get_rank<A>>(I, J))>(std::forward<A>(a));
    }

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

} // namespace enda
