#include "TestCommon.hpp"

TEST(PrintTest, LayoutPropStream)
{
    auto              p = static_cast<enda::layout_prop_e>((1 << 0) | (1 << 1));
    std::stringstream ss;
    ss << p;
    std::string expected = "contiguous   "
                           "strided_1d   "
                           "smallest_stride_is_one   ";
    EXPECT_EQ(ss.str(), expected);
}

TEST(PrintTest, PrintIdxMapStream)
{
    enda::idx_map<1, 0, 0, static_cast<enda::layout_prop_e>(0)> im;
    std::stringstream                                           ss;
    ss << im;
    std::string out = ss.str();
    EXPECT_FALSE(out.empty());
}

TEST(PrintTest, Print1DArray)
{
    enda::array<long, 1>       a {1, 2, 3};
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "[1,2,3]");
}

TEST(PrintTest, Print2DArray)
{
    enda::array<long, 2>       a {{1, 2}, {3, 4}};
    std::stringstream ss;
    ss << a;
    std::string expected = "\n[[1,2]\n [3,4]]";
    EXPECT_EQ(ss.str(), expected);
}

TEST(PrintTest, Print3DArray)
{
    enda::array<long, 3> a {{{1, 2, 3}, {4, 5, 6}}, {{7, 8, 9}, {10, 11, 12}}};
    std::stringstream ss;
    ss << a;
    EXPECT_EQ(ss.str(), "[1,2,3,4,5,6,7,8,9,10,11,12]");
}
