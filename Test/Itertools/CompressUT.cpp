#include <Itertools/Compress.hpp>
#include <gtest/gtest.h>
#include <vector>

// Helper function to convert an iterable range to a vector.
template<typename Range>
auto to_vector(Range&& range)
{
    using ValueType = std::decay_t<decltype(*std::begin(range))>;
    std::vector<ValueType> result;
    for (auto&& element : range)
    {
        result.push_back(element);
    }
    return result;
}

//------------------------------------------------------------------------------
// Test when both the input vector and selectors are empty.
//------------------------------------------------------------------------------
TEST(CompressTest, BothEmpty)
{
    std::vector<int>  ints {};
    std::vector<bool> selectors {};
    auto              result = to_vector(enda::itertools::compress(ints.begin(), ints.end(), selectors.begin(), selectors.end()));
    std::vector<int>  expected {};
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test when the input vector is non-empty but selectors are empty.
//------------------------------------------------------------------------------
TEST(CompressTest, NonEmptyIntsEmptySelectors)
{
    std::vector<int>  ints {1, 2};
    std::vector<bool> selectors {};
    auto              result = to_vector(enda::itertools::compress(ints.begin(), ints.end(), selectors.begin(), selectors.end()));
    std::vector<int>  expected {}; // No selectors means no elements are selected.
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test when the input vector is empty but selectors are non-empty.
//------------------------------------------------------------------------------
TEST(CompressTest, EmptyIntsNonEmptySelectors)
{
    std::vector<int>  ints {};
    std::vector<bool> selectors {true, true};
    auto              result = to_vector(enda::itertools::compress(ints.begin(), ints.end(), selectors.begin(), selectors.end()));
    std::vector<int>  expected {}; // No input elements, so the result is empty.
    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test the normal case when both containers have equal lengths.
//------------------------------------------------------------------------------
TEST(CompressTest, NormalCaseExactLength)
{
    std::vector<int>  ints {1, 2, 3, 4};
    std::vector<bool> selectors {false, true, false, true};
    auto              result = to_vector(enda::itertools::compress(ints.begin(), ints.end(), selectors.begin(), selectors.end()));
    std::vector<int>  expected {2, 4};
    EXPECT_EQ(result, expected);
}
