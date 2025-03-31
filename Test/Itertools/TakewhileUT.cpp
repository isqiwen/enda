#include <Itertools/Takewhile.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for takewhile: verifies that only elements satisfying the predicate
// (n < 3) are taken from the beginning of the vector.
//------------------------------------------------------------------------------
TEST(TakewhileTest, BasicTakewhile)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    std::vector<int> result;

    // Use takewhile with a predicate that takes elements while n < 3.
    for (auto n : enda::itertools::takewhile([](int n) { return n < 3; }, nums))
    {
        result.push_back(n);
    }

    // Expected result: only 1 and 2 are taken.
    std::vector<int> expected {1, 2};
    EXPECT_EQ(result, expected);
}
