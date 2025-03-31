#include <Itertools/Islice.hpp>
#include <gtest/gtest.h>
#include <list>
#include <string>
#include <utility>

//------------------------------------------------------------------------------
// Helper function that takes a container (or string) and returns a string
// containing the concatenated characters produced by islice with the
// specified start, stop, and step values.
//------------------------------------------------------------------------------
template<typename Items>
std::string islice_to_string(Items&& items, int start, int stop, int step)
{
    std::string result;
    for (auto c : enda::itertools::islice(std::forward<Items>(items), start, stop, step))
    {
        result.push_back(c);
    }
    return result;
}

//------------------------------------------------------------------------------
// Test case for islice with a std::list<char>.
// The list contains "0123456789abcdef" and we slice from index 1 to 16 with a step of 2.
// Expected output: "13579bdf"
//------------------------------------------------------------------------------
TEST(IsliceTest, ListCharTest)
{
    std::list<char> lst    = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    std::string     result = islice_to_string(lst, 1, 16, 2);
    EXPECT_EQ(result, "13579bdf");
}

//------------------------------------------------------------------------------
// Test cases for islice with a std::string ("ABCDEFG").
//------------------------------------------------------------------------------
TEST(IsliceTest, StringStep1)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 0, 7, 1);
    EXPECT_EQ(result, "ABCDEFG");
}

TEST(IsliceTest, StringStep2)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 0, 7, 2);
    // Expected: indices 0,2,4,6 => "A", "C", "E", "G"
    EXPECT_EQ(result, "ACEG");
}

TEST(IsliceTest, StringStep3)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 0, 7, 3);
    // Expected: indices 0,3,6 => "A", "D", "G"
    EXPECT_EQ(result, "ADG");
}

TEST(IsliceTest, StringStep4)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 0, 7, 4);
    // Expected: indices 0,4 => "A", "E"
    EXPECT_EQ(result, "AE");
}

TEST(IsliceTest, StringStep7)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 0, 7, 7);
    // Expected: index 0 only => "A"
    EXPECT_EQ(result, "A");
}

TEST(IsliceTest, StringStartOutOfRange)
{
    std::string s      = "ABCDEFG";
    std::string result = islice_to_string(s, 7, 14, 1);
    // Since start index equals the string length, expect an empty string.
    EXPECT_EQ(result, "");
}

TEST(IsliceTest, EmptyString)
{
    std::string s      = "";
    std::string result = islice_to_string(s, 0, 7, 1);
    EXPECT_EQ(result, "");
}
