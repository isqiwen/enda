#include <array>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include <Itertools/Product.hpp>

using namespace enda::itertools;

// Test the product of two ranges using product().
TEST(ProductTest, TwoRanges)
{
    std::vector<int>  v1 {1, 2, 3};
    std::vector<char> v2 {'a', 'b'};

    // Create a cartesian product range from v1 and v2.
    auto prod = product(v1, v2);

    // Collect the results into a vector of tuples.
    std::vector<std::tuple<int, char>> result;
    for (auto const& tup : prod)
    {
        result.push_back(tup);
    }

    // Expected product:
    // (1, 'a'), (1, 'b'),
    // (2, 'a'), (2, 'b'),
    // (3, 'a'), (3, 'b')
    std::vector<std::tuple<int, char>> expected {{1, 'a'}, {1, 'b'}, {2, 'a'}, {2, 'b'}, {3, 'a'}, {3, 'b'}};

    EXPECT_EQ(result, expected);
}

// Test the product of three ranges.
TEST(ProductTest, ThreeRanges)
{
    std::vector<int>    v1 {1, 2};
    std::vector<char>   v2 {'x', 'y'};
    std::vector<double> v3 {0.1, 0.2};

    // Create a product range with three ranges.
    auto prod = product(v1, v2, v3);

    // Collect the results.
    std::vector<std::tuple<int, char, double>> result;
    for (auto const& tup : prod)
    {
        result.push_back(tup);
    }

    // Expected combinations: 2*2*2 = 8 tuples.
    std::vector<std::tuple<int, char, double>> expected = {
        {1, 'x', 0.1}, {1, 'x', 0.2}, {1, 'y', 0.1}, {1, 'y', 0.2}, {2, 'x', 0.1}, {2, 'x', 0.2}, {2, 'y', 0.1}, {2, 'y', 0.2}};

    EXPECT_EQ(result, expected);
}

// Test the use of make_product with an array of ranges.
TEST(ProductTest, MakeProductFromArray)
{
    std::vector<int> v1 {10, 20};
    std::vector<int> v2 {30, 40};

    // Create an array of ranges.
    std::array ranges = {v1, v2};

    // Use make_product to create the cartesian product range.
    auto prod = make_product(ranges);

    // Collect the results.
    std::vector<std::tuple<int, int>> result;
    for (auto const& tup : prod)
    {
        result.push_back(tup);
    }

    // Expected product:
    // (10, 30), (10, 40),
    // (20, 30), (20, 40)
    std::vector<std::tuple<int, int>> expected {{10, 30}, {10, 40}, {20, 30}, {20, 40}};

    EXPECT_EQ(result, expected);
}

// Test the iterator and sentinel comparison behavior.
// We check that iterating until the sentinel (end) stops at the correct position.
TEST(ProductTest, IteratorSentinelComparison)
{
    std::vector<int>  v1 {5, 6};
    std::vector<char> v2 {'A', 'B', 'C'};
    auto              prod = product(v1, v2);

    // Use a manual loop to count the number of iterations.
    int count = 0;
    for (auto it = prod.begin(); it != prod.end(); ++it)
    {
        ++count;
    }
    // Expected count: 2 * 3 = 6
    EXPECT_EQ(count, 6);
}

// Test that dereferencing returns the correct tuple.
TEST(ProductTest, Dereference)
{
    std::vector<int>  v1 {7};
    std::vector<char> v2 {'z'};
    auto              prod = product(v1, v2);

    // Get the first (and only) tuple.
    auto it  = prod.begin();
    auto tup = *it; // should be (7, 'z')
    EXPECT_EQ(std::get<0>(tup), 7);
    EXPECT_EQ(std::get<1>(tup), 'z');
}
