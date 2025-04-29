/**
 * @file MatrixFunctions.hpp
 *
 * @brief Provides functions to create and manipulate matrices, i.e. arrays/view with 'M' algebra.
 */

#pragma once

#include <algorithm>
#include <concepts>
#include <ranges>
#include <type_traits>

#include "Accessors.hpp"
#include "Concepts.hpp"
#include "Declarations.hpp"
#include "Layout/Policies.hpp"
#include "LayoutTransforms.hpp"
#include "Macros.hpp"
#include "MappedFunctions.hpp"
#include "Mem/Policies.hpp"
#include "StdUtil/Array.hpp"

namespace enda
{
    template<Scalar S, std::integral Int = long>
    auto eye(Int dim)
    {
        auto r = matrix<S>(dim, dim);
        r      = S {1};
        return r;
    }

    template<ArrayOfRank<2> M>
    auto trace(M const& m)
    {
        static_assert(get_rank<M> == 2, "Error in enda::trace: Array/View must have rank 2");
        EXPECTS(m.shape()[0] == m.shape()[1]);
        auto r = get_value_t<M> {};
        auto d = m.shape()[0];
        for (int i = 0; i < d; ++i)
            r += m(i, i);
        return r;
    }

    template<ArrayOfRank<2> M>
    ArrayOfRank<2> auto dagger(M const& m)
    {
        if constexpr (is_complex_v<typename M::value_type>)
            return conj(transpose(m));
        else
            return transpose(m);
    }

    template<MemoryArrayOfRank<2> M>
    ArrayOfRank<1> auto diagonal(M& m)
    {
        long dim            = std::min(m.shape()[0], m.shape()[1]);
        long stride         = stdutil::sum(m.indexmap().strides());
        using vector_view_t = basic_array_view<std::remove_reference_t<decltype(*m.data())>, 1, C_stride_layout, 'V', enda::default_accessor, enda::borrowed<>>;
        return vector_view_t {C_stride_layout::mapping<1> {{dim}, {stride}}, m.data()};
    }

    template<typename V>
    requires(std::ranges::contiguous_range<V> or ArrayOfRank<V, 1>) ArrayOfRank<2> auto diag(V const& v)
    {
        if constexpr (std::ranges::contiguous_range<V>)
        {
            return diag(enda::basic_array_view {v});
        }
        else
        {
            auto m      = matrix<std::remove_const_t<typename V::value_type>>::zeros({v.size(), v.size()});
            diagonal(m) = v;
            return m;
        }
    }

    template<ArrayOfRank<2> A, ArrayOfRank<2> B>
    requires(std::same_as<get_value_t<A>, get_value_t<B>>) // NB the get_value_t gets rid of const if any
        matrix<get_value_t<A>> vstack(A const& a, B const& b)
    {
        static_assert(get_rank<A> == 2, "Error in enda::vstack: Only rank 2 arrays/views are allowed");
        static_assert(get_rank<A> == 2, "Error in enda::vstack: Only rank 2 arrays/views are allowed");
        EXPECTS_WITH_MESSAGE(a.shape()[1] == b.shape()[1], "Error in enda::vstack: The second dimension of the two matrices must be equal");

        auto [n, q]              = a.shape();
        auto                   p = b.shape()[0];
        matrix<get_value_t<A>> res(n + p, q);
        res(range(0, n), range::all)     = a;
        res(range(n, n + p), range::all) = b;
        return res;
    }

} // namespace enda
