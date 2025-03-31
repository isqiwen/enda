#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

#include <Itertools/CombinationsWithReplacement.hpp>
#include <Itertools/RangeView.hpp>

//------------------------------------------------------------------------------
// Helper function to convert a range of 1-element tuples to a vector<int>.
//------------------------------------------------------------------------------
std::vector<int> to_vector_1(auto&& range)
{
    std::vector<int> result;
    for (auto&& [a] : range)
    {
        result.push_back(a);
    }
    return result;
}

//------------------------------------------------------------------------------
// Helper function to convert a range of 3-element tuples to a vector of vectors.
//------------------------------------------------------------------------------
std::vector<std::vector<int>> to_vector_of_vectors_3(auto&& range)
{
    std::vector<std::vector<int>> result;
    for (auto&& [a, b, c] : range)
    {
        result.push_back({a, b, c});
    }
    return result;
}

//------------------------------------------------------------------------------
// Helper function to convert a range of 5-element tuples to a vector of vectors.
//------------------------------------------------------------------------------
std::vector<std::vector<int>> to_vector_of_vectors_5(auto&& range)
{
    std::vector<std::vector<int>> result;
    for (auto&& [a, b, c, d, e] : range)
    {
        result.push_back({a, b, c, d, e});
    }
    return result;
}

//------------------------------------------------------------------------------
// Test case: combinations_with_replacement for choosing 1 element.
//------------------------------------------------------------------------------
TEST(CombinationsWithReplacementTest, Choose1)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    // Using combinations_with_replacement with k = 1
    auto             range    = enda::itertools::combinations_with_replacement<1>(nums.begin(), nums.end());
    auto             result   = to_vector_1(range);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test case: combinations_with_replacement for choosing 3 elements.
//------------------------------------------------------------------------------
TEST(CombinationsWithReplacementTest, Choose3)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    // k = 3 yields all non-decreasing triplets from nums.
    auto range  = enda::itertools::combinations_with_replacement<3>(nums.begin(), nums.end());
    auto result = to_vector_of_vectors_3(range);

    // There are 35 combinations when choosing 3 elements from 5 with replacement.
    EXPECT_EQ(result.size(), 35);
    ASSERT_FALSE(result.empty());
    // Verify the first and last combinations.
    EXPECT_EQ(result.front(), (std::vector<int> {1, 1, 1}));
    EXPECT_EQ(result.back(), (std::vector<int> {5, 5, 5}));
}

//------------------------------------------------------------------------------
// Test case: combinations_with_replacement for choosing 5 elements.
//------------------------------------------------------------------------------
TEST(CombinationsWithReplacementTest, Choose5)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    // k = 5 yields all non-decreasing 5-element combinations.
    auto range  = enda::itertools::combinations_with_replacement<5>(nums.begin(), nums.end());
    auto result = to_vector_of_vectors_5(range);

    // Expected count is (n+k-1 choose k) = (5+5-1 choose 5) = (9 choose 5) = 126.
    EXPECT_EQ(result.size(), 126);
    ASSERT_FALSE(result.empty());
    // Verify the first combination is all ones and the last is all fives.
    EXPECT_EQ(result.front(), (std::vector<int> {1, 1, 1, 1, 1}));
    EXPECT_EQ(result.back(), (std::vector<int> {5, 5, 5, 5, 5}));
}

//------------------------------------------------------------------------------
// Test case: combinations_with_replacement for choosing 6 elements.
// This test verifies that no combinations are generated for k greater than the number
// of elements, as indicated by the original code's comment.
//------------------------------------------------------------------------------
TEST(CombinationsWithReplacementTest, Choose6Empty)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    auto             range = enda::itertools::combinations_with_replacement<6>(nums.begin(), nums.end());
    int              count = 0;
    for (auto&& tup : range)
    {
        (void)tup; // suppress unused variable warning
        ++count;
    }
    // As per the provided sample, expecting zero combinations.
    EXPECT_EQ(count, 0);
}
