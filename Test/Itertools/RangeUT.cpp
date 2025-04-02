#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <gtest/gtest.h>

#include <Itertools/Range.hpp>

using namespace enda::itertools;

// Test basic functionality of range with a single parameter.
TEST(RangeTest, SingleParameter)
{
    // range(last): should generate integers from 0 to last-1.
    range             r(5);
    std::vector<long> result;
    for (auto i : r)
    {
        result.push_back(i);
    }
    std::vector<long> expected = {0, 1, 2, 3, 4};
    EXPECT_EQ(result, expected);
}

// Test basic functionality of range with two parameters.
TEST(RangeTest, TwoParameters)
{
    // range(first, last): should generate integers from first to last-1.
    range             r(-2, 1);
    std::vector<long> result;
    for (auto i : r)
    {
        result.push_back(i);
    }
    std::vector<long> expected = {-2, -1, 0};
    EXPECT_EQ(result, expected);
}

// Test basic functionality of range with three parameters.
TEST(RangeTest, ThreeParameters)
{
    {
        // range(10, 3, -2) should generate: 10, 8, 6, 4.
        range             r(10, 3, -2);
        std::vector<long> result;
        for (auto i : r)
        {
            result.push_back(i);
        }
        std::vector<long> expected = {10, 8, 6, 4};
        EXPECT_EQ(result, expected);
    }
    {
        // range(0, 10, -1) with negative step and increasing interval should be empty.
        range             r(0, 10, -1);
        std::vector<long> result;
        for (auto i : r)
        {
            result.push_back(i);
        }
        std::vector<long> expected = {};
        EXPECT_EQ(result, expected);
    }
}

// Test the output stream operator for range.
TEST(RangeTest, OutputOperator)
{
    range              r(3, 10, 2);
    std::ostringstream oss;
    oss << r;
    std::string expected = "range(3,10,2)";
    EXPECT_EQ(oss.str(), expected);
}

// Test the foreach function to apply a callable to every element.
TEST(RangeTest, ForEachFunction)
{
    std::ostringstream oss;
    // Using foreach to print squares of numbers from 1 to 10.
    foreach (range(1, 11), [&oss](int i) { oss << (i * i) << " "; })
        ;
    std::string expected = "1 4 9 16 25 36 49 64 81 100 ";
    EXPECT_EQ(oss.str(), expected);
}

// Test product_range with integer arguments.
// This should construct ranges: range(2) and range(3) and produce their cartesian product.
TEST(RangeTest, ProductRangeIntegers)
{
    auto                                prod = product_range(2, 3);
    std::vector<std::tuple<long, long>> result;
    for (auto const& tup : prod)
    {
        result.push_back(tup);
    }
    // Expected product: (0,0), (0,1), (0,2), (1,0), (1,1), (1,2)
    std::vector<std::tuple<long, long>> expected = {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}};
    EXPECT_EQ(result, expected);
}

// Test product_range with a tuple of integers.
TEST(RangeTest, ProductRangeTuple)
{
    auto                                tup  = std::make_tuple(2, 3);
    auto                                prod = product_range(tup);
    std::vector<std::tuple<long, long>> result;
    for (auto const& elem : prod)
    {
        result.push_back(elem);
    }
    std::vector<std::tuple<long, long>> expected = {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}};
    EXPECT_EQ(result, expected);
}

// Test product_range with an array of integers.
TEST(RangeTest, ProductRangeArray)
{
    std::array<int, 2>                  arr  = {2, 3};
    auto                                prod = product_range(arr);
    std::vector<std::tuple<long, long>> result;
    for (auto const& tup : prod)
    {
        result.push_back(tup);
    }
    std::vector<std::tuple<long, long>> expected = {{0, 0}, {0, 1}, {0, 2}, {1, 0}, {1, 1}, {1, 2}};
    EXPECT_EQ(result, expected);
}
