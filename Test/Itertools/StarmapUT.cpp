#include <Itertools/Starmap.hpp>
#include <gtest/gtest.h>
#include <tuple>
#include <vector>

//------------------------------------------------------------------------------
// Function to compute power: my_pow(x, n) returns x^n.
//------------------------------------------------------------------------------
int my_pow(int x, int n)
{
    if (n == 0)
    {
        return 1;
    }
    int h = my_pow(x, n / 2);
    return h * h * (n % 2 ? x : 1);
}

//------------------------------------------------------------------------------
// Test case for starmap: apply my_pow to a vector of argument tuples.
//------------------------------------------------------------------------------
TEST(StarmapTest, MyPowApplication)
{
    // Create a vector of tuples with pairs (x, n)
    std::vector<std::tuple<int, int>> args {{2, 5}, {3, 2}, {10, 3}};

    // Collect results from starmap into a vector
    std::vector<int> result;
    for (auto res : enda::itertools::starmap(my_pow, args))
    {
        result.push_back(res);
    }

    // Expected values: 2^5 = 32, 3^2 = 9, 10^3 = 1000
    std::vector<int> expected {32, 9, 1000};
    EXPECT_EQ(result, expected);
}
