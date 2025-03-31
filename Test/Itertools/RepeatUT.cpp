#include <Itertools/RangeView.hpp>
#include <Itertools/Repeat.hpp>
#include <gtest/gtest.h>
#include <string>
#include <vector>

//------------------------------------------------------------------------------
// Test case for repeat: verifies that repeating "Knock, knock" exactly 5 times
// and "Hello there" indefinitely works as expected.
//------------------------------------------------------------------------------
TEST(RepeatTest, FiniteAndInfiniteRepeat)
{
    // Create an infinite repeater for "Hello there"
    auto infinite_repeater = enda::itertools::repeat("Hello there").begin();

    // Create a finite repeater for "Knock, knock" repeated 5 times
    std::vector<std::pair<std::string, std::string>> result;
    for (auto const& msg : enda::itertools::repeat("Knock, knock", 5))
    {
        // Pre-increment the infinite repeater and collect the pair
        std::string first  = msg;
        std::string second = *(++infinite_repeater);
        result.push_back({first, second});
    }

    // Expected: 5 pairs of ("Knock, knock", "Hello there")
    std::vector<std::pair<std::string, std::string>> expected(5, {"Knock, knock", "Hello there"});
    EXPECT_EQ(result, expected);
}
