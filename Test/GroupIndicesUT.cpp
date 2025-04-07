#include "TestCommon.hpp"

TEST(GroupIndicesTest, ValidPartitionTest)
{
    // Test the is_partition_of_indices function.
    // Define two groups: one containing indices {0, 1} and the other containing index {2}.
    constexpr std::array<int, 2> grp1 = {0, 1};
    constexpr std::array<int, 1> grp2 = {2};

    // Here, R = 3.
    bool valid = detail::is_partition_of_indices<3>(grp1, grp2);
    EXPECT_TRUE(valid);
}

TEST(GroupIndicesTest, InvalidPartitionTest)
{
    // Test that when there are duplicate or missing indices, an exception is thrown.
    // For instance, groups {0, 0} and {2} for R=3 are invalid (duplicate and missing index 1).
    constexpr std::array<int, 2> grp1 = {0, 0};
    constexpr std::array<int, 1> grp2 = {2};

    // Expect the function to throw a runtime_error when duplicate indices are detected.
    EXPECT_FALSE(detail::is_partition_of_indices<3>(grp1, grp2));
}

TEST(GroupIndicesTest, StrideOrderOfGroupedIdxMapTest)
{
    // Test for Rank=3 with a non-natural stride order so that the static stride_order is {2, 1, 0}.
    // The original idx_map's static stride_order is {2, 1, 0}.
    constexpr std::array<int, 3> orig_stride_order = {2, 1, 0};
    // Simulate two groups: group1 = {0, 1} and group2 = {2}.
    constexpr std::array<int, 2> grp1 = {0, 1};
    constexpr std::array<int, 1> grp2 = {2};

    // Calculation explanation:
    // 1. Compute mem_pos = inverse(orig_stride_order).
    //    For orig_stride_order = {2, 1, 0}, mem_pos = {2, 1, 0}.
    // 2. For group1, the corresponding mem_pos values are { mem_pos[0]=2, mem_pos[1]=1 }
    //    => min = 1, max = 2, so the indices are consecutive.
    // 3. For group2, the corresponding mem_pos value is { mem_pos[2]=0 } => min = 0.
    // 4. The min_mem_positions array is {1, 0}.
    // 5. Ranking the groups by their slowest varying index yields mem_pos_out = {1, 0}.
    // 6. The new stride order is inverse({1,0}) = {1, 0}.
    auto               new_stride_order = detail::stride_order_of_grouped_idx_map<3>(orig_stride_order, grp1, grp2);
    std::array<int, 2> expected         = {1, 0};
    EXPECT_EQ(new_stride_order, expected);
}

TEST(GroupIndicesTest, GroupIndicesLayoutValidTest)
{
    // Use group_indices_layout to construct a new idx_map.
    // Assume the original idx_map has Rank=3, extents = {2, 3, 4}, corresponding to a contiguous array
    // (e.g., col-major order). For testing, we use StrideOrder = 18 (i.e., static stride_order is {2,1,0}) and layout is contiguous.
    using orig_map_t                 = idx_map<3, 0, 18, layout_prop_e::contiguous>;
    std::array<long, 3> orig_extents = {2, 3, 4};
    // For a contiguous array, typically strides = {12, 4, 1}.
    std::array<long, 3> orig_strides = {1, 2, 6};
    orig_map_t          orig_map(orig_extents, orig_strides);

    // Grouping: combine dimensions 0 and 1 into one group, with dimension 2 as a separate group.
    // Here, idx_group<0,1> and idx_group<2> become std::array<int,2>{0,1} and std::array<int,1>{2} respectively.
    auto new_map = group_indices_layout<3, 0, 18, layout_prop_e::contiguous>(orig_map, idx_group<0, 1>, idx_group<2>);

    // Expected results:
    // New rank = 2.
    // New extents: first group extent = extents[0] * extents[1] = 2 * 3 = 6, second group extent = extents[2] = 4.
    std::array<long, 2> expected_extents = {6, 4};

    // New strides: first group stride = min(strides[0], strides[1]) = min(12, 4) = 4, second group stride = strides[2] = 1.
    std::array<long, 2> expected_strides = {1, 6};

    // Based on the previous test, the new static stride order should be {1, 0}.
    constexpr std::array<int, 2> expected_stride_order = {1, 0};

    // Validate the new idx_map.
    EXPECT_EQ(new_map.lengths(), expected_extents);
    EXPECT_EQ(new_map.strides(), expected_strides);
    // Validate the static stride order.
    EXPECT_EQ(decltype(new_map)::stride_order, expected_stride_order);
}
