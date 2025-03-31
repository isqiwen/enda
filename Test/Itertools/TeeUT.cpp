#include <Itertools/Tee.hpp>
#include <algorithm>
#include <gtest/gtest.h>
#include <vector>

//------------------------------------------------------------------------------
// Test case for tee: verifies that tee produces two iterators with identical output.
//------------------------------------------------------------------------------
TEST(TeeTest, ProducesEqualSequences)
{
    std::vector<int> nums {1, 2, 3, 4, 5};

    // Create two independent iterators using tee.
    auto [X, Y] = enda::itertools::tee<2>(nums);

    // Verify that both iterators yield the same sequence.
    EXPECT_TRUE(std::equal(X.begin(), X.end(), Y.begin(), Y.end()));
}
