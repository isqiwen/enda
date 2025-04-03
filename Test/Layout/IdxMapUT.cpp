#include <gtest/gtest.h>

#include "Enda.hpp"

#include "../GtestTools.hpp"

using namespace enda;
using enda::slice_static::slice_idx_map;

// Test a fully static idx_map (e.g. a 2D array with shape [3, 4]) using C-order (row-major) layout.
TEST(IdxMapTest, FullyStatic)
{
    constexpr int Rank = 2;
    // Static extents: dimension 0 = 3, dimension 1 = 4
    constexpr uint64_t staticExtents = encode(std::array<int, Rank> {3, 4});
    // C-order: use the predefined C_stride_order (i.e. identity permutation encoded)
    constexpr uint64_t strideOrder = C_stride_order<Rank>;
    // Construct an idx_map with default layout properties.
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m;

    // Check shape (lengths)
    auto lengths = m.lengths();
    EXPECT_EQ(lengths[0], 3);
    EXPECT_EQ(lengths[1], 4);

    // For a 2D C-order array, the expected strides are: dimension 0 (row) = 4, dimension 1 (column) = 1
    auto strides = m.strides();
    EXPECT_EQ(strides[0], 4);
    EXPECT_EQ(strides[1], 1);

    // Check total number of elements and compile-time size.
    EXPECT_EQ(m.size(), 12);
    EXPECT_EQ(m.ce_size(), 12);

    // Test mapping from multi-index to linear index.
    // In row-major order: m(i, j) = i * 4 + j
    EXPECT_EQ(m(0, 0), 0);
    EXPECT_EQ(m(0, 1), 1);
    EXPECT_EQ(m(1, 0), 4);
    EXPECT_EQ(m(2, 3), 11);

    // Test reverse mapping: linear index 7 should map back to (1, 3)
    auto idx = m.to_idx(7);
    EXPECT_EQ(idx[0], 1);
    EXPECT_EQ(idx[1], 3);

    // For contiguous C-order layout, is_contiguous() and is_strided_1d() should return true.
    EXPECT_TRUE(m.is_contiguous());
    EXPECT_TRUE(m.is_strided_1d());
}

// Test a partially dynamic idx_map (e.g. 2D array where dimension 0 is dynamic and dimension 1 is fixed as 4)
TEST(IdxMapTest, PartiallyDynamic)
{
    constexpr int Rank = 2;
    // Static extents: dimension 0 is dynamic (0), dimension 1 is fixed to 4
    constexpr uint64_t staticExtents = encode(std::array<int, Rank> {0, 4});
    constexpr uint64_t strideOrder   = C_stride_order<Rank>; // Use C-order
    // When constructing, pass the dynamic extents (only one dynamic dimension): assume dimension 0 is actually 3.
    std::array<long, 1>                                                  dyn = {3};
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m(dyn);

    auto lengths = m.lengths();
    EXPECT_EQ(lengths[0], 3);
    EXPECT_EQ(lengths[1], 4);

    // For a C-order 2D array, expected strides remain: dimension 0 = 4, dimension 1 = 1.
    auto strides = m.strides();
    EXPECT_EQ(strides[0], 4);
    EXPECT_EQ(strides[1], 1);
    EXPECT_EQ(m.size(), 12);
}

// Test constructing an idx_map from a given shape (fully dynamic)
TEST(IdxMapTest, FromShape)
{
    constexpr int Rank = 3;
    // Fully dynamic: all dimensions are 0 at compile time.
    constexpr uint64_t staticExtents = encode(std::array<int, Rank> {0, 0, 0});
    constexpr uint64_t strideOrder   = C_stride_order<Rank>;
    // Provide a shape: [2, 3, 4]
    std::array<long, Rank>                                               shape = {2, 3, 4};
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m(shape);

    auto lengths = m.lengths();
    EXPECT_EQ(lengths[0], 2);
    EXPECT_EQ(lengths[1], 3);
    EXPECT_EQ(lengths[2], 4);

    // For a 3D C-order array, the expected contiguous strides are:
    // Dimension 0: stride = 3*4 = 12, dimension 1: stride = 4, dimension 2: stride = 1.
    auto strides = m.strides();
    EXPECT_EQ(strides[0], 12);
    EXPECT_EQ(strides[1], 4);
    EXPECT_EQ(strides[2], 1);
    EXPECT_EQ(m.size(), 24);
}

// Test multi-index mapping (operator()) and reverse mapping (to_idx) correctness.
TEST(IdxMapTest, MultiIndexMapping)
{
    constexpr int                                                        Rank          = 3;
    constexpr uint64_t                                                   staticExtents = encode(std::array<int, Rank> {2, 3, 4});
    constexpr uint64_t                                                   strideOrder   = C_stride_order<Rank>;
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m;

    // For a 2x3x4 array in C-order, the mapping is: m(i, j, k) = i*12 + j*4 + k.
    EXPECT_EQ(m(0, 0, 0), 0);
    EXPECT_EQ(m(0, 0, 1), 1);
    EXPECT_EQ(m(0, 1, 0), 4);
    EXPECT_EQ(m(1, 0, 0), 12);
    EXPECT_EQ(m(1, 2, 3), 23);

    // Reverse mapping test: linear index 17 should correspond to (1, 1, 1)
    auto idx = m.to_idx(17);
    EXPECT_EQ(idx[0], 1);
    EXPECT_EQ(idx[1], 1);
    EXPECT_EQ(idx[2], 1);
}

// Test the transpose operation: for a 2D array, transposing should swap rows and columns.
TEST(IdxMapTest, Transpose)
{
    constexpr int                                                        Rank          = 2;
    constexpr uint64_t                                                   staticExtents = encode(std::array<int, Rank> {3, 4});
    constexpr uint64_t                                                   strideOrder   = C_stride_order<Rank>;
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m;
    // For a 2D array, swapping the two dimensions corresponds to a permutation {1, 0}.
    constexpr uint64_t permutation = encode(std::array<int, Rank> {1, 0});
    auto               mt          = m.transpose<permutation>();

    // The original m has shape [3,4]; after transposition, it should become [4,3].
    auto new_lengths = mt.lengths();
    EXPECT_EQ(new_lengths[0], 4);
    EXPECT_EQ(new_lengths[1], 3);

    // For a C-order layout, original strides are [4,1]; after transposition, they should become [1,4].
    auto new_strides = mt.strides();
    EXPECT_EQ(new_strides[0], m.strides()[1]);
    EXPECT_EQ(new_strides[1], m.strides()[0]);

    // Check that the mapping is correctly transposed: m(2, 3) should equal mt(3, 2).
    EXPECT_EQ(m(2, 3), mt(3, 2));
}

// Test the slice interface (assuming slice_static::slice_idx_map returns a valid result).
TEST(IdxMapTest, Slice)
{
    constexpr int                                                        Rank          = 3;
    constexpr uint64_t                                                   staticExtents = encode(std::array<int, Rank> {4, 5, 6});
    constexpr uint64_t                                                   strideOrder   = C_stride_order<Rank>;
    idx_map<Rank, staticExtents, strideOrder, layout_prop_e::contiguous> m;

    // Call the slice() interface.
    // For example: slice dimension 0 to [1,3), dimension 1 to all (using range::all), and dimension 2 at index 4.
    auto slice_result = m.slice(range(1, 3), range::all, 4);
    // The returned value is a pair: first is the offset (flat index) and second is the new idx_map.
    // Here we mainly test that the interface can be called without throwing and the new map's rank matches the original.
    EXPECT_EQ(slice_result.second.rank(), 2);
    // Assume that the offset is non-negative.
    EXPECT_GE(slice_result.first, 34);
}

template<typename... INT>
std::array<long, sizeof...(INT)> ma(INT... i)
{
    return {i...};
}

//-----------------------

TEST(IdxMapTest, Construct)
{
    idx_map<3, 0, C_stride_order<3>, layout_prop_e::none> i1 {{1, 2, 3}};

    std::cerr << i1 << std::endl;
    EXPECT_TRUE(i1.lengths() == (ma(1, 2, 3)));
    EXPECT_TRUE(i1.strides() == (ma(6, 3, 1)));
}

//-----------------------

TEST(IdxMapTest, eval)
{
    idx_map<3, 0, C_stride_order<3>, layout_prop_e::none> i1 {{2, 7, 3}};
    EXPECT_TRUE(i1.strides() == (ma(21, 3, 1)));

    EXPECT_EQ(i1(1, 3, 2), 21 * 1 + 3 * 3 + 2 * 1);
}

//-------------------------

TEST(IdxMapTest, to_idx)
{
    idx_map<3, 0, C_stride_order<3>, layout_prop_e::none>       iC {{2, 7, 3}};
    idx_map<3, 0, Fortran_stride_order<3>, layout_prop_e::none> iF {{2, 7, 3}};
    auto                                                        iP = iF.transpose<encode(std::array {0, 2, 1})>();

    for (auto idx0 : range(2))
    {
        auto iPv = iP.slice(idx0, _, _).second;

        for (auto idx1 : range(7))
        {
            for (auto idx2 : range(3))
            {
                EXPECT_TRUE(iPv.to_idx(iPv(idx2, idx1)) == ma(idx2, idx1));
                EXPECT_TRUE(iC.to_idx(iC(idx0, idx1, idx2)) == ma(idx0, idx1, idx2));
                EXPECT_TRUE(iF.to_idx(iF(idx0, idx1, idx2)) == ma(idx0, idx1, idx2));
                EXPECT_TRUE(iP.to_idx(iP(idx0, idx2, idx1)) == ma(idx0, idx2, idx1));
            }
        }
    }
}

TEST(IdxMapTest, slicemat)
{
    idx_map<2, 0, C_stride_order<2>, layout_prop_e::none> i1 {{10, 10}};

    auto [offset2, i2] = slice_idx_map(i1, range(0, 2), 2);

    static_assert(decltype(i2)::layout_prop == layout_prop_e::strided_1d, "000");
}

//-----------------------

TEST(IdxMapTest, slice)
{
    idx_map<3, 0, C_stride_order<3>, layout_prop_e::none> i1 {{1, 2, 3}};

    auto [offset2, i2] = slice_idx_map(i1, 0, _, 2);

    idx_map<1, 0, C_stride_order<1>, layout_prop_e::strided_1d> c2 {{2}, {3}};

    std::cerr << i2 << std::endl;
    std::cerr << c2 << std::endl;
    EXPECT_TRUE(i2 == c2);
    EXPECT_EQ(offset2, 2);

    auto [offset3, i3] = slice_idx_map(i1, _, _, _);
    EXPECT_TRUE(i3 == i1);
    EXPECT_EQ(offset3, 0);
}

//-----------------------

TEST(IdxMapTest, ellipsis)
{
    EXPECT_EQ(16, encode(enda::slice_static::detail::slice_stride_order(std::array<int, 3> {0, 1, 2}, std::array<int, 2> {1, 2})));

    idx_map<3, 0, C_stride_order<3>, layout_prop_e::none> i1 {{1, 2, 3}};
    auto [offset2, i2] = slice_idx_map(i1, 0, ___);

    idx_map<2, 0, C_stride_order<2>, layout_prop_e::none> c2 {{2, 3}, {3, 1}};

    std::cerr << i2 << std::endl;
    std::cerr << c2 << std::endl;
    EXPECT_TRUE(i2 == c2);
    EXPECT_EQ(offset2, 0);

    auto [offset3, i3] = slice_idx_map(i1, ___);
    EXPECT_TRUE(i3 == i1);
    EXPECT_EQ(offset3, 0);
}

//-----------------------

TEST(IdxMapTest, ellipsis2)
{
    idx_map<5, 0, C_stride_order<5>, layout_prop_e::none> i1 {{1, 2, 3, 4, 5}};
    std::cerr << i1 << std::endl;

    auto [offset2, i2] = slice_idx_map(i1, 0, ___, 3, 2);
    idx_map<2, 0, C_stride_order<2>, layout_prop_e::none> c2 {{2, 3}, {60, 20}};

    std::cerr << i2 << std::endl;
    std::cerr << c2 << std::endl;
    EXPECT_TRUE(i2 == c2);
    EXPECT_EQ(offset2, i1(0, 0, 0, 3, 2));
}

TEST(IdxMapTest, for_each)
{
    {
        std::stringstream fs;
        auto              l = [&fs](int i, int j, int k) { fs << i << j << k << " "; };

        for_each(std::array<long, 3> {1, 2, 3}, l);
        EXPECT_EQ(fs.str(), "000 001 002 010 011 012 ");
    }
}
