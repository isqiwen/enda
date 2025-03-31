#include <Itertools/Permutations.hpp>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test permutations of length 1.
//------------------------------------------------------------------------------
TEST(PermutationsTest, PermutationsOfLength1)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    std::vector<int> result;

    // Each tuple has one element.
    for (auto&& tup : enda::itertools::permutations<1>(nums))
    {
        auto [a] = tup;
        result.push_back(a);
    }

    // Expected output: {1, 2, 3, 4, 5}
    std::vector<int> expected {1, 2, 3, 4, 5};
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test permutations of length 3.
//------------------------------------------------------------------------------
TEST(PermutationsTest, PermutationsOfLength3)
{
    std::vector<int> nums {1, 2, 3, 4, 5};
    int              count = 0;

    // There should be 5P3 = 5*4*3 = 60 permutations.
    for (auto&& [a, b, c] : enda::itertools::permutations<3>(nums))
    {
        // Verify that the permutation contains distinct elements.
        EXPECT_NE(a, b);
        EXPECT_NE(a, c);
        EXPECT_NE(b, c);
        ++count;
    }
    EXPECT_EQ(count, 60);
}
