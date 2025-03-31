#include <gtest/gtest.h>
#include <iterator>
#include <tuple>
#include <vector>

#include <Itertools/Combinations.hpp>
#include <Itertools/RangeView.hpp>

// Helper function: returns a vector of vectors from an iterable of tuples (for 3-element combinations).
std::vector<std::vector<int>> to_vector_of_vectors_3(auto&& range)
{
    std::vector<std::vector<int>> result;
    for (auto&& [a, b, c] : range)
    {
        result.push_back({a, b, c});
    }
    return result;
}

// Helper function: returns a vector from an iterable of 1-element tuples.
std::vector<int> to_vector_1(auto&& range)
{
    std::vector<int> result;
    for (auto&& [a] : range)
    {
        result.push_back(a);
    }
    return result;
}

// Helper function: returns a vector of vectors from an iterable of 5-element tuples.
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
// Test combinations_iterator for 5 choose 3 using range_view
//------------------------------------------------------------------------------
TEST(CombinationsIteratorTest, FiveChooseThree)
{
    std::vector<int> nums {1, 2, 3, 4, 5};

    // Construct the begin iterator for combinations of 3 from the first 3 elements.
    auto first = enda::itertools::combinations_iterator<3, decltype(nums.begin())>(nums.begin(), nums.begin() + 3);
    // Construct the end iterator. (Based on the library's convention, the end is built with both iterators equal.)
    auto last = enda::itertools::combinations_iterator<3, decltype(nums.begin())>(nums.begin() + 3, nums.begin() + 3);

    auto                          result   = to_vector_of_vectors_3(enda::itertools::range_view(first, last));
    std::vector<std::vector<int>> expected = {{1, 2, 3}, {1, 2, 4}, {1, 2, 5}, {1, 3, 4}, {1, 3, 5}, {1, 4, 5}, {2, 3, 4}, {2, 3, 5}, {2, 4, 5}, {3, 4, 5}};
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test combinations function for various combination sizes.
//------------------------------------------------------------------------------
TEST(CombinationsTest, Choose1)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    auto             range    = enda::itertools::combinations<1>(nums.begin(), nums.end());
    auto             result   = to_vector_1(range);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(result, expected);
}

TEST(CombinationsTest, Choose3)
{
    std::vector<int>              nums {1, 2, 3, 4, 5};
    auto                          range    = enda::itertools::combinations<3>(nums.begin(), nums.end());
    auto                          result   = to_vector_of_vectors_3(range);
    std::vector<std::vector<int>> expected = {{1, 2, 3}, {1, 2, 4}, {1, 2, 5}, {1, 3, 4}, {1, 3, 5}, {1, 4, 5}, {2, 3, 4}, {2, 3, 5}, {2, 4, 5}, {3, 4, 5}};
    EXPECT_EQ(result, expected);
}

TEST(CombinationsTest, Choose5)
{
    std::vector<int>              nums {1, 2, 3, 4, 5};
    auto                          range    = enda::itertools::combinations<5>(nums.begin(), nums.end());
    auto                          result   = to_vector_of_vectors_5(range);
    std::vector<std::vector<int>> expected = {{1, 2, 3, 4, 5}};
    EXPECT_EQ(result, expected);
}

TEST(CombinationsTest, Choose6Empty)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    auto             range = enda::itertools::combinations<6>(nums.begin(), nums.end());
    // Expect no combinations when choosing 6 out of 5 elements.
    int count = 0;
    for (auto&& tup : range)
    {
        (void)tup; // suppress unused variable warning
        ++count;
    }
    EXPECT_EQ(count, 0);
}
