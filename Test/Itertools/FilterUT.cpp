#include <Itertools/Filter.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for filter: verifies that filter returns only truthy values from the input.
// In this example, non-zero integers are considered true, so the expected result is {1, -1, 1}.
//------------------------------------------------------------------------------
TEST(FilterTest, BasicFilter)
{
    std::vector<int> nums {1, 0, -1, 0, 1};

    // Collect the results of applying filter to nums.
    std::vector<int> result;
    for (auto n : enda::itertools::filter(nums))
    {
        result.push_back(n);
    }

    // Expected output: {1, -1, 1}
    std::vector<int> expected {1, -1, 1};
    EXPECT_EQ(result, expected);
}
