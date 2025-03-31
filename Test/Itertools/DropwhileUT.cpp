#include <Itertools/Dropwhile.hpp>
#include <gtest/gtest.h>
#include <vector>


//------------------------------------------------------------------------------
// Test case for dropwhile: verifies that all initial elements satisfying the predicate
// are dropped and the rest are returned.
//------------------------------------------------------------------------------
TEST(DropwhileTest, Basic)
{
    // Create a vector of integers.
    std::vector<int> nums {1, 2, 3, 4, 5};

    // Use dropwhile with a predicate that drops values less than 3.
    auto range = enda::itertools::dropwhile([](int n) { return n < 3; }, nums);

    // Collect the remaining elements into a vector.
    std::vector<int> result;
    for (auto n : range)
    {
        result.push_back(n);
    }

    // Expected output: {3, 4, 5}
    std::vector<int> expected {3, 4, 5};
    EXPECT_EQ(result, expected);
}
