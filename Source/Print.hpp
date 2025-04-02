/**
 * @file Print.hpp
 *
 * @brief Provides various overloads of the operator<< for nda related objects.
 */

#pragma once

#include <cstdint>
#include <ostream>

#include "Arithmetic.hpp"
#include "ArrayAdapter.hpp"
#include "Concepts.hpp"
#include "Layout/IdxMap.hpp"
#include "Layout/Permutation.hpp"
#include "Map.hpp"
#include "Traits.hpp"

namespace enda
{

    /**
     * @addtogroup layout_utils
     * @{
     */

    /**
     * @brief Write an enda::layout_prop_e to a std::ostream.
     *
     * @param sout std::ostream object.
     * @param p enda::layout_prop_e object.
     * @return Reference to std::ostream object.
     */
    inline std::ostream& operator<<(std::ostream& sout, layout_prop_e p)
    {
        return sout << (has_contiguous(p) ? "contiguous   " : " ") << (has_strided_1d(p) ? "strided_1d   " : " ")
                    << (has_smallest_stride_is_one(p) ? "smallest_stride_is_one   " : " ");
    }

    /**
     * @brief Write an enda::idx_map to a std::ostream.
     *
     * @tparam Rank Rank of the enda::idx_map.
     * @tparam StaticExtents StaticExtents of the enda::idx_map.
     * @tparam StrideOrder StrideOrder of the enda::idx_map.
     * @tparam LayoutProp Layout property of the enda::idx_map.
     * @param sout std::ostream object.
     * @param idxm enda::idx_map object.
     * @return Reference to std::ostream object.
     */
    template<int Rank, uint64_t StaticExtents, uint64_t StrideOrder, layout_prop_e LayoutProp>
    std::ostream& operator<<(std::ostream& sout, idx_map<Rank, StaticExtents, StrideOrder, LayoutProp> const& idxm)
    {
        return sout << "  Lengths  : " << idxm.lengths() << "\n"
                    << "  Strides  : " << idxm.strides() << "\n"
                    << "  StaticExtents  : " << decode<Rank>(StaticExtents) << "\n"
                    << "  MemoryStrideOrder   : " << idxm.stride_order << "\n"
                    << "  Flags   :  " << LayoutProp << "\n";
    }

    /** @} */

    /**
     * @addtogroup av_utils
     * @{
     */

    /**
     * @brief Write an enda::basic_array or enda::basic_array_view to a std::ostream.
     *
     * @tparam A Type of the enda::basic_array or enda::basic_array_view.
     * @param sout std::ostream object.
     * @param a enda::basic_array or enda::basic_array_view object.
     * @return Reference to std::ostream object.
     */
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

    /**
     * @brief Write an enda::array_adapter to a std::ostream.
     *
     * @tparam R Rank of the enda::array_adapter.
     * @tparam F Callable type of the enda::array_adapter.
     * @param sout std::ostream object.
     * @param aa enda::array_adapter object.
     * @return Reference to std::ostream object.
     */
    template<int R, typename F>
    std::ostream& operator<<(std::ostream& sout, array_adapter<R, F> const& aa)
    {
        return sout << "array_adapter of shape " << aa.shape();
    }

    /// @cond
    // Forward declarations (necessary for libclang parsing).
    template<char OP, Array A>
    struct expr_unary;

    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    struct expr;
    /// @endcond

    /**
     * @brief Write an enda::expr_unary to a std::ostream.
     *
     * @tparam OP Unary operator.
     * @tparam A enda::Array type.
     * @param sout std::ostream object.
     * @param ex enda::expr_unary object.
     * @return Reference to std::ostream object.
     */
    template<char OP, Array A>
    std::ostream& operator<<(std::ostream& sout, expr_unary<OP, A> const& ex)
    {
        return sout << OP << ex.a;
    }

    /**
     * @brief Write an enda::expr to a std::ostream.
     *
     * @tparam OP Binary operator.
     * @tparam L enda::ArrayOrScalar type of left hand side.
     * @tparam R enda::ArrayOrScalar type of right hand side.
     * @param sout std::ostream object.
     * @param ex enda::expr object.
     * @return Reference to std::ostream object.
     */
    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    std::ostream& operator<<(std::ostream& sout, expr<OP, L, R> const& ex)
    {
        return sout << "(" << ex.l << " " << OP << " " << ex.r << ")";
    }

    /**
     * @brief Write an enda::expr_call to a std::ostream.
     *
     * @tparam F Callable type.
     * @tparam As Argument types.
     * @param sout std::ostream object.
     * @return Reference to std::ostream object.
     */
    template<typename F, typename... As>
    std::ostream& operator<<(std::ostream& sout, expr_call<F, As...> const&)
    {
        return sout << "mapped"; // array<value_type, std::decay_t<A>::rank>(x);
    }

    /** @} */

} // namespace enda
