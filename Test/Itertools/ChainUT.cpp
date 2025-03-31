#include <gtest/gtest.h>
#include <list>
#include <vector>

#include <Itertools/Chain.hpp>
#include <Itertools/RangeView.hpp>

//------------------------------------------------------------------------------
// Test case for chain_iterator using range_view
//------------------------------------------------------------------------------
TEST(ChainIteratorTest, ConcatenatesTwoSequences)
{
    // Create a vector of integers
    std::vector<int> ints_vec {1, 2, 3, 4};
    // Create a list of integers
    std::list<int> ints_list {5, 6, 7};

    // Construct chain_iterator manually from vector and list iterators
    auto first = enda::itertools::chain_iterator<int, decltype(ints_vec.begin()), decltype(ints_list.begin())>(
        ints_vec.begin(), ints_vec.end(), ints_list.begin(), ints_list.end());
    auto last = enda::itertools::chain_iterator<int, decltype(ints_vec.begin()), decltype(ints_list.begin())>(
        ints_vec.end(), ints_vec.end(), ints_list.end(), ints_list.end());

    // Use range_view to iterate over the concatenated sequence
    std::vector<int> result;
    for (auto i : enda::itertools::range_view(first, last))
    {
        result.push_back(i);
    }

    // Expected sequence: [1, 2, 3, 4, 5, 6, 7]
    std::vector<int> expected {1, 2, 3, 4, 5, 6, 7};
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test case for chain() function
//------------------------------------------------------------------------------
TEST(ChainTest, ConcatenatesTwoSequences)
{
    // Create a vector of integers
    std::vector<int> ints_vec {1, 2, 3, 4};
    // Create a list of integers
    std::list<int> ints_list {5, 6, 7};

    // Use the chain function to concatenate the two containers
    auto chain_range = enda::itertools::chain(ints_vec, ints_list);

    // Collect the results into a vector
    std::vector<int> result;
    for (auto i : chain_range)
    {
        result.push_back(i);
    }

    // Expected sequence: [1, 2, 3, 4, 5, 6, 7]
    std::vector<int> expected {1, 2, 3, 4, 5, 6, 7};
    EXPECT_EQ(result, expected);
}
