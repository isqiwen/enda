/**
 * @file BasicFunctions.hpp
 *
 * @brief Provides basic functions to create and manipulate arrays and views.
 */

#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <random>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Declarations.hpp"
#include "Exceptions.hpp"
#include "Itertools/Itertools.hpp"
#include "Layout/ForEach.hpp"
#include "Mem/AddressSpace.hpp"
#include "Traits.hpp"

namespace enda
{
    template<typename T, mem::AddressSpace AdrSp = mem::Host, std::integral Int, auto Rank>
    auto zeros(std::array<Int, Rank> const& shape)
    {
        static_assert(AdrSp != mem::None);
        if constexpr (Rank == 0)
            return T {};
        else if constexpr (AdrSp == mem::Host)
            return array<T, Rank>::zeros(shape);
        else
            return cuarray<T, Rank>::zeros(shape);
    }

    template<typename T, mem::AddressSpace AdrSp = mem::Host, std::integral... Ints>
    auto zeros(Ints... is)
    {
        return zeros<T, AdrSp>(std::array<long, sizeof...(Ints)> {static_cast<long>(is)...});
    }

    template<typename T, std::integral Int, auto Rank>
    auto ones(std::array<Int, Rank> const& shape) requires(enda::is_scalar_v<T>)
    {
        if constexpr (Rank == 0)
            return T {1};
        else
        {
            return array<T, Rank>::ones(shape);
        }
    }

    template<typename T, std::integral... Ints>
    auto ones(Ints... is)
    {
        return ones<T>(std::array<long, sizeof...(Ints)> {is...});
    }

    template<std::integral Int = long>
    auto arange(long first, long last, long step = 1)
    {
        auto r = range(first, last, step);
        auto a = array<Int, 1>(r.size());
        for (auto [x, v] : itertools::zip(a, r))
            x = v;
        return a;
    }

    template<std::integral Int = long>
    auto arange(long last)
    {
        return arange<Int>(0, last);
    }

    template<typename RealType = double, std::integral Int, auto Rank>
    auto rand(std::array<Int, Rank> const& shape) requires(std::is_floating_point_v<RealType>)
    {
        if constexpr (Rank == 0)
        {
            auto static gen  = std::mt19937 {};
            auto static dist = std::uniform_real_distribution<> {0.0, 1.0};
            return dist(gen);
        }
        else
        {
            return array<RealType, Rank>::rand(shape);
        }
    }

    template<typename RealType = double, std::integral... Ints>
    auto rand(Ints... is)
    {
        return rand<RealType>(std::array<long, sizeof...(Ints)> {is...});
    }

    template<Array A>
    long first_dim(A const& a)
    {
        return a.extent(0);
    }

    template<Array A>
    long second_dim(A const& a)
    {
        return a.extent(1);
    }

    template<typename A, typename A_t = std::decay_t<A>>
    decltype(auto) make_regular(A&& a)
    {
        if constexpr (Array<A> and not is_regular_v<A>)
        {
            return basic_array {std::forward<A>(a)};
        }
        else if constexpr (requires { typename A_t::regular_t; })
        {
            if constexpr (not std::is_same_v<A_t, typename A_t::regular_t>)
                return typename A_t::regular_t {std::forward<A>(a)};
            else
                return std::forward<A>(a);
        }
        else
        {
            return std::forward<A>(a);
        }
    }

    template<MemoryArray A>
    decltype(auto) to_host(A&& a)
    {
        if constexpr (not mem::on_host<A>)
        {
            return get_regular_host_t<A> {std::forward<A>(a)};
        }
        else
        {
            return std::forward<A>(a);
        }
    }

    template<MemoryArray A>
    decltype(auto) to_device(A&& a)
    {
        if constexpr (not mem::on_device<A>)
        {
            return get_regular_device_t<A> {std::forward<A>(a)};
        }
        else
        {
            return std::forward<A>(a);
        }
    }

    template<MemoryArray A>
    decltype(auto) to_unified(A&& a)
    {
        if constexpr (not mem::on_unified<A>)
        {
            return get_regular_unified_t<A> {std::forward<A>(a)};
        }
        else
        {
            return std::forward<A>(a);
        }
    }

    template<typename A>
    void resize_or_check_if_view(A& a, std::array<long, A::rank> const& sha) requires(is_regular_or_view_v<A>)
    {
        if (a.shape() == sha)
            return;
        if constexpr (is_regular_v<A>)
        {
            a.resize(sha);
        }
        else
        {
            ENDA_RUNTIME_ERROR << "Error in enda::resize_or_check_if_view: Size mismatch: " << a.shape() << " != " << sha;
        }
    }

    template<typename T, int R, typename LP, char A, typename CP>
    auto make_const_view(basic_array<T, R, LP, A, CP> const& a)
    {
        return basic_array_view<T const, R, LP, A> {a};
    }

    template<typename T, int R, typename LP, char A, typename AP, typename OP>
    auto make_const_view(basic_array_view<T, R, LP, A, AP, OP> const& a)
    {
        return basic_array_view<T const, R, LP, A, AP, OP> {a};
    }

    template<typename T, int R, typename LP, char A, typename CP>
    auto make_array_view(basic_array<T, R, LP, A, CP> const& a)
    {
        return array_view<T, R> {a};
    }

    template<typename T, int R, typename LP, char A, typename AP, typename OP>
    auto make_array_view(basic_array_view<T, R, LP, A, AP, OP> const& a)
    {
        return array_view<T, R> {a};
    }

    template<typename T, int R, typename LP, char A, typename CP>
    auto make_array_const_view(basic_array<T, R, LP, A, CP> const& a)
    {
        return array_const_view<T, R> {a};
    }

    template<typename T, int R, typename LP, char A, typename AP, typename OP>
    auto make_array_const_view(basic_array_view<T, R, LP, A, AP, OP> const& a)
    {
        return array_const_view<T, R> {a};
    }

    template<typename T, int R, typename LP, char A, typename CP>
    auto make_matrix_view(basic_array<T, R, LP, A, CP> const& a)
    {
        return matrix_view<T, LP> {a};
    }

    template<typename T, int R, typename LP, char A, typename AP, typename OP>
    auto make_matrix_view(basic_array_view<T, R, LP, A, AP, OP> const& a)
    {
        return matrix_view<T, LP> {a};
    }

    template<Array LHS, Array RHS>
    bool operator==(LHS const& lhs, RHS const& rhs)
    {
        if (lhs.shape() != rhs.shape())
            return false;
        bool r = true;
        enda::for_each(lhs.shape(), [&](auto&&... x) { r &= (lhs(x...) == rhs(x...)); });
        return r;
    }

    template<ArrayOfRank<1> A, std::ranges::contiguous_range R>
    bool operator==(A const& a, R const& rg)
    {
        return a == basic_array_view {rg};
    }

    template<std::ranges::contiguous_range R, ArrayOfRank<1> A>
    bool operator==(R const& rg, A const& a)
    {
        return a == rg;
    }

    template<MemoryArray A>
    auto get_block_layout(A const& a)
    {
        EXPECTS(!a.empty());
        using opt_t = std::optional<std::tuple<int, int, int>>;

        auto const& shape   = a.indexmap().lengths();
        auto const& strides = a.indexmap().strides();
        auto const& order   = a.indexmap().stride_order;

        int data_size  = shape[order[0]] * strides[order[0]];
        int block_size = data_size;
        int block_str  = data_size;
        int n_blocks   = 1;

        for (auto n : range(A::rank))
        {
            auto inner_size = (n == A::rank - 1) ? 1 : strides[order[n + 1]] * shape[order[n + 1]];
            if (strides[order[n]] != inner_size)
            {
                if (block_size < data_size) // second strided dimension
                    return opt_t {};
                // found a strided dimension with (assumed) contiguous inner blocks
                n_blocks   = a.size() / inner_size;
                block_size = inner_size;
                block_str  = strides[order[n]];
            }
        }
        ASSERT(n_blocks * block_size == a.size());
        ASSERT(n_blocks * block_str == shape[order[0]] * strides[order[0]]);
        return opt_t {std::make_tuple(n_blocks, block_size, block_str)};
    }

    template<size_t Axis = 0, Array A0, Array... As>
    auto concatenate(A0 const& a0, As const&... as)
    {
        // sanity checks
        auto constexpr rank = A0::rank;
        static_assert(Axis < rank);
        static_assert(have_same_rank_v<A0, As...>);
        static_assert(have_same_value_type_v<A0, As...>);
        for (auto ax [[maybe_unused]] : range(rank))
        {
            EXPECTS(ax == Axis or ((a0.extent(ax) == as.extent(ax)) and ... and true));
        }

        // construct concatenated array
        auto new_shape  = a0.shape();
        new_shape[Axis] = (as.extent(Axis) + ... + new_shape[Axis]);
        auto new_array  = array<get_value_t<A0>, rank>(new_shape);

        // slicing helper function
        auto slice_Axis = [](Array auto& a, range r) {
            auto all_or_range = std::make_tuple(range::all, r);
            return [&]<auto... Is>(std::index_sequence<Is...>) { return a(std::get < Is == Axis > (all_or_range)...); }(std::make_index_sequence<rank> {});
        };

        // initialize concatenated array
        long offset = 0;
        for (auto const& a_view : {basic_array_view(a0), basic_array_view(as)...})
        {
            slice_Axis(new_array, range(offset, offset + a_view.extent(Axis))) = a_view;
            offset += a_view.extent(Axis);
        }

        return new_array;
    };

} // namespace enda
