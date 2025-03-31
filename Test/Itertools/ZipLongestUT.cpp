#include <Itertools/ZipLongest.hpp>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include <tuple>
#include <vector>

//------------------------------------------------------------------------------
// Test case for zip_longest: verifies that zip_longest produces tuples by
// combining elements from three containers, using default values for exhausted ones.
// For this example, ints = {1,2,3,4,5}, doubles = {0.1,0.2,0.3}, and letters = "ABCDEFG".
// The longest container is letters (7 elements), so the expected tuples are:
//   (1, 0.1, 'A'), (2, 0.2, 'B'), (3, 0.3, 'C'),
//   (4, 0.0, 'D'), (5, 0.0, 'E'), (0, 0.0, 'F'), (0, 0.0, 'G').
//------------------------------------------------------------------------------
TEST(ZipLongestTest, BasicZipLongest)
{
    std::vector<int>  ints {1, 2, 3, 4, 5};
    std::list<double> doubles {0.1, 0.2, 0.3};
    std::string       letters {"ABCDEFG"};

    std::vector<std::tuple<int, double, char>> result;
    for (auto [a, b, c] : enda::itertools::zip_longest(ints, doubles, letters))
    {
        result.push_back({a, b, c});
    }

    std::vector<std::tuple<int, double, char>> expected = {
        {1, 0.1, 'A'}, {2, 0.2, 'B'}, {3, 0.3, 'C'}, {4, 0.0, 'D'}, {5, 0.0, 'E'}, {0, 0.0, 'F'}, {0, 0.0, 'G'}};

    EXPECT_EQ(result, expected);
}
