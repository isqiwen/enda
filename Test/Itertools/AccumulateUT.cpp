#include <Itertools/Accumulate.hpp>
#include <gtest/gtest.h>
#include <vector>

TEST(AccumulateTest, MultipleElements)
{
    std::vector<int> v            = {1, 2, 3, 4, 5};
    auto             result_range = enda::itertools::accumulate(v.begin(), v.end(), 0);

    std::vector<int> result;
    for (const auto& x : result_range)
    {
        result.push_back(x);
    }

    std::vector<int> expected = {1, 3, 6, 10, 15};
    EXPECT_EQ(result, expected);
}

TEST(AccumulateTest, EmptyVector)
{
    std::vector<int> v;
    auto             result_range = enda::itertools::accumulate(v.begin(), v.end(), 0);

    std::vector<int> result;
    for (auto&& x : result_range)
    {
        result.push_back(x);
    }

    std::vector<int> expected = {0};
    EXPECT_EQ(result, expected);
}

TEST(AccumulateTest, SingleElement)
{
    std::vector<int> v            = {1};
    auto             result_range = enda::itertools::accumulate(v.begin(), v.end(), 0);

    std::vector<int> result;
    for (auto&& x : result_range)
    {
        result.push_back(x);
    }

    std::vector<int> expected = {1};
    EXPECT_EQ(result, expected);
}
