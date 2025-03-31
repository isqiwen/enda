#include <Itertools/Groupby.hpp>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

//------------------------------------------------------------------------------
// Helper function to convert the groupby output into a vector of pairs,
// where each pair consists of a key and a vector of grouped elements.
//------------------------------------------------------------------------------
template<typename T>
std::vector<std::pair<T, std::vector<T>>> groupby_to_vector(std::vector<T>& nums)
{
    std::vector<std::pair<T, std::vector<T>>> result;
    for (auto [key, group] : enda::itertools::groupby(nums))
    {
        std::vector<T> grp;
        for (auto element : group)
        {
            grp.push_back(element);
        }
        result.emplace_back(key, grp);
    }
    return result;
}

//------------------------------------------------------------------------------
// Test groupby with an empty vector.
//------------------------------------------------------------------------------
TEST(GroupByTest, EmptyVector)
{
    std::vector<int> nums {};
    auto             groups = groupby_to_vector(nums);
    EXPECT_TRUE(groups.empty());
}

//------------------------------------------------------------------------------
// Test groupby with a single element vector.
//------------------------------------------------------------------------------
TEST(GroupByTest, SingleElement)
{
    std::vector<int>                              nums {1};
    auto                                          groups   = groupby_to_vector(nums);
    std::vector<std::pair<int, std::vector<int>>> expected = {{1, {1}}};
    EXPECT_EQ(groups, expected);
}

//------------------------------------------------------------------------------
// Test groupby with all elements the same.
//------------------------------------------------------------------------------
TEST(GroupByTest, AllSameElements)
{
    std::vector<int>                              nums {3, 3, 3};
    auto                                          groups   = groupby_to_vector(nums);
    std::vector<std::pair<int, std::vector<int>>> expected = {{3, {3, 3, 3}}};
    EXPECT_EQ(groups, expected);
}

//------------------------------------------------------------------------------
// Test groupby with multiple groups in sorted order.
//------------------------------------------------------------------------------
TEST(GroupByTest, MultipleGroupsSorted)
{
    std::vector<int>                              nums {1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5};
    auto                                          groups   = groupby_to_vector(nums);
    std::vector<std::pair<int, std::vector<int>>> expected = {{1, {1}}, {2, {2, 2}}, {3, {3, 3, 3}}, {4, {4, 4, 4, 4}}, {5, {5, 5, 5, 5, 5}}};
    EXPECT_EQ(groups, expected);
}

//------------------------------------------------------------------------------
// Test groupby with non-sorted input, which groups adjacent equal elements.
//------------------------------------------------------------------------------
TEST(GroupByTest, MultipleGroupsNonSorted)
{
    std::vector<int>                              nums {1, 2, 2, 3, 3, 3, 2, 2, 1};
    auto                                          groups   = groupby_to_vector(nums);
    std::vector<std::pair<int, std::vector<int>>> expected = {{1, {1}}, {2, {2, 2}}, {3, {3, 3, 3}}, {2, {2, 2}}, {1, {1}}};
    EXPECT_EQ(groups, expected);
}
