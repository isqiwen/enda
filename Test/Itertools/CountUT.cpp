#include <Itertools/Count.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for count_iterator
//------------------------------------------------------------------------------
TEST(CountIteratorTest, BasicCounting)
{
    // Create a count iterator that starts at 1 and increments by 2.
    auto counter = enda::itertools::count_iterator<int, int>(1, 2);

    // Collect the first 5 values from the iterator.
    std::vector<int> result;
    for (int i = 0; i < 5; ++i, ++counter)
    {
        result.push_back(*counter);
    }

    // Expected sequence: 1, 3, 5, 7, 9.
    std::vector<int> expected = {1, 3, 5, 7, 9};
    EXPECT_EQ(result, expected);
}
