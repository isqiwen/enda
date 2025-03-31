#include <Itertools/Filterfalse.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for filterfalse: verifies that filterfalse returns only the elements
// for which the predicate evaluates to false (i.e. falsey values).
// In this example, non-zero integers are considered true and 0 is considered false.
//------------------------------------------------------------------------------
TEST(FilterfalseTest, FiltersOutTruthyValues)
{
    // Create a vector of integers.
    std::vector<int> nums {1, 0, -1, 0, 1};

    // Collect the result of filterfalse.
    std::vector<int> result;
    for (auto n : enda::itertools::filterfalse(nums))
    {
        result.push_back(n);
    }

    // Expected result: only the false values (i.e. zeros) remain.
    std::vector<int> expected {0, 0};
    EXPECT_EQ(result, expected);
}
