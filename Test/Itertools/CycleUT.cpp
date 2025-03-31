#include <Itertools/Cycle.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for cycle_iterator: verifies that cycling over the sequence {1, 2, 3}
// produces the expected output when iterated 7 times.
//------------------------------------------------------------------------------
TEST(CycleIteratorTest, CycleSevenElements)
{
    std::vector<int> nums {1, 2, 3};
    auto             cycler = enda::itertools::cycle_iterator(nums.begin(), nums.end());

    std::vector<int> result;
    for (int i = 0; i < 7; ++i, ++cycler)
    {
        result.push_back(*cycler);
    }

    // Expected sequence: 1, 2, 3, 1, 2, 3, 1
    std::vector<int> expected {1, 2, 3, 1, 2, 3, 1};
    EXPECT_EQ(result, expected);
}
