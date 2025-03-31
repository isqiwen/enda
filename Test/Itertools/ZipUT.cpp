#include <Itertools/Zip.hpp>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include <tuple>
#include <vector>

//------------------------------------------------------------------------------
// Test case for zip: verifies that zip stops at the shortest container.
// For ints = {1,2,3,4,5}, doubles = {0.1, 0.2, 0.3} and letters = "ABCDEFG",
// the expected tuples are: (1, 0.1, 'A'), (2, 0.2, 'B'), (3, 0.3, 'C').
//------------------------------------------------------------------------------
TEST(ZipTest, BasicZip)
{
    std::vector<int>  ints {1, 2, 3, 4, 5};
    std::list<double> doubles {0.1, 0.2, 0.3};
    std::string       letters {"ABCDEFG"};

    std::vector<std::tuple<int, double, char>> result;
    for (auto [x, y, z] : enda::itertools::zip(ints, doubles, letters))
    {
        result.push_back({x, y, z});
    }

    std::vector<std::tuple<int, double, char>> expected = {{1, 0.1, 'A'}, {2, 0.2, 'B'}, {3, 0.3, 'C'}};
    EXPECT_EQ(result, expected);
}
