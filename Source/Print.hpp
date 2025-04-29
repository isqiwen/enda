/**
 * @file Print.hpp
 *
 * @brief Provides various overloads of the operator<< for nda related objects.
 */

#pragma once

#include <cstdint>
#include <ostream>

#include "Arithmetic.hpp"
#include "Concepts.hpp"
#include "Layout/IdxMap.hpp"
#include "Layout/Permutation.hpp"
#include "Map.hpp"
#include "Traits.hpp"

namespace enda
{
    inline std::ostream& operator<<(std::ostream& sout, layout_prop_e p)
    {
        return sout << (has_contiguous(p) ? "contiguous   " : " ") << (has_strided_1d(p) ? "strided_1d   " : " ")
                    << (has_smallest_stride_is_one(p) ? "smallest_stride_is_one   " : " ");
    }

    template<int Rank, uint64_t StaticExtents, uint64_t StrideOrder, layout_prop_e LayoutProp>
    std::ostream& operator<<(std::ostream& sout, idx_map<Rank, StaticExtents, StrideOrder, LayoutProp> const& idxm)
    {
        return sout << "  Lengths  : " << idxm.lengths() << "\n"
                    << "  Strides  : " << idxm.strides() << "\n"
                    << "  StaticExtents  : " << decode<Rank>(StaticExtents) << "\n"
                    << "  MemoryStrideOrder   : " << idxm.stride_order << "\n"
                    << "  Flags   :  " << LayoutProp << "\n";
    }

    template<typename A>
    std::ostream& operator<<(std::ostream& sout, A const& a) requires(is_regular_or_view_v<A>)
    {
        // 1-dimensional array/view
        if constexpr (A::rank == 1)
        {
            sout << "[";
            auto const& len = a.indexmap().lengths();
            for (size_t i = 0; i < len[0]; ++i)
                sout << (i > 0 ? "," : "") << a(i);
            sout << "]";
        }

        // 2-dimensional array/view
        if constexpr (A::rank == 2)
        {
            auto const& len = a.indexmap().lengths();
            sout << "\n[";
            for (size_t i = 0; i < len[0]; ++i)
            {
                sout << (i == 0 ? "[" : " [");
                for (size_t j = 0; j < len[1]; ++j)
                    sout << (j > 0 ? "," : "") << a(i, j);
                sout << "]" << (i == len[0] - 1 ? "" : "\n");
            }
            sout << "]";
        }

        // FIXME : not very pretty, do better here, but that was the arrays way
        // higher-dimensional array/view (flat representation)
        if constexpr (A::rank > 2)
        {
            sout << "[";
            for (bool first = true; auto& v : a)
            {
                sout << (first ? "" : ",") << v;
                first = false;
            }
            sout << "]";
        }

        return sout;
    }

    template<char OP, Array A>
    struct expr_unary;

    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    struct expr;

    template<char OP, Array A>
    std::ostream& operator<<(std::ostream& sout, expr_unary<OP, A> const& ex)
    {
        return sout << OP << ex.a;
    }

    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    std::ostream& operator<<(std::ostream& sout, expr<OP, L, R> const& ex)
    {
        return sout << "(" << ex.l << " " << OP << " " << ex.r << ")";
    }

    template<typename F, typename... As>
    std::ostream& operator<<(std::ostream& sout, expr_call<F, As...> const&)
    {
        return sout << "mapped"; // array<value_type, std::decay_t<A>::rank>(x);
    }

} // namespace enda
