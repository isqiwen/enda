#include "../TestCommon.hpp"

using namespace enda::stdutil;

TEST(ArrayUtilTest, ToString)
{
    std::array<int, 3> arr = {1, 2, 3};
    EXPECT_EQ(to_string(arr), "(1 2 3)");
}

TEST(ArrayUtilTest, StreamOperator)
{
    std::array<int, 3> arr = {4, 5, 6};
    std::stringstream  ss;
    ss << arr;
    EXPECT_EQ(ss.str(), "(4 5 6)");
}

TEST(ArrayUtilTest, Addition)
{
    std::array<int, 3> a        = {1, 2, 3};
    std::array<int, 3> b        = {4, 5, 6};
    std::array<int, 3> expected = {5, 7, 9};
    EXPECT_EQ(a + b, expected);
}

TEST(ArrayUtilTest, Subtraction)
{
    std::array<int, 3> a        = {7, 8, 9};
    std::array<int, 3> b        = {1, 2, 3};
    std::array<int, 3> expected = {6, 6, 6};
    EXPECT_EQ(a - b, expected);
}

TEST(ArrayUtilTest, ScalarMultiplication)
{
    std::array<int, 3> a        = {1, 2, 3};
    int                scalar   = 2;
    std::array<int, 3> expected = {2, 4, 6};
    EXPECT_EQ(a * scalar, expected);
}

TEST(ArrayUtilTest, Append)
{
    std::array<int, 3> a        = {1, 2, 3};
    int                b        = 4;
    auto               result   = append(a, b);
    std::array<int, 4> expected = {1, 2, 3, 4};
    EXPECT_EQ(result, expected);
}

TEST(ArrayUtilTest, FrontAppend)
{
    std::array<int, 2> a        = {1, 2};
    int                b        = 3;
    auto               result   = front_append(a, b);
    std::array<int, 3> expected = {3, 1, 2};
    EXPECT_EQ(result, expected);
}

TEST(ArrayUtilTest, Pop)
{
    std::array<int, 4> a        = {10, 20, 30, 40};
    auto               result   = pop(a);
    std::array<int, 3> expected = {10, 20, 30};
    EXPECT_EQ(result, expected);
}

TEST(ArrayUtilTest, FrontMPop)
{
    std::array<int, 5> a        = {1, 2, 3, 4, 5};
    auto               result   = front_mpop<2>(a);
    std::array<int, 3> expected = {3, 4, 5};
    EXPECT_EQ(result, expected);
}

TEST(ArrayUtilsTest, Join)
{
    std::array<int, 3> a1       = {1, 2, 3};
    std::array<int, 2> a2       = {4, 5};
    std::array<int, 5> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(join(a1, a2), expected);
}

TEST(ArrayUtilsTest, Sum)
{
    std::array<int, 4> arr = {1, 2, 3, 4};
    EXPECT_EQ(sum(arr), 10);

    std::array<double, 3> arrD = {1.5, 2.5, 3.0};
    EXPECT_DOUBLE_EQ(sum(arrD), 7.0);

    std::array<int, 0> emptyArr = {};
    EXPECT_EQ(sum(emptyArr), 0);
}

TEST(ArrayUtilsTest, Product)
{
    std::array<int, 3> arr = {2, 3, 4};
    EXPECT_EQ(product(arr), 24);

    std::array<double, 2> arrD = {1.5, 2.0};
    EXPECT_DOUBLE_EQ(product(arrD), 3.0);
}

TEST(ArrayUtilsTest, DotProduct)
{
    std::array<int, 3> a1 = {1, 2, 3};
    std::array<int, 3> a2 = {4, 5, 6};
    EXPECT_EQ(dot_product(a1, a2), 32);

    std::array<double, 2> a3 = {1.5, 2.5};
    std::array<double, 2> a4 = {3.0, 4.0};
    EXPECT_DOUBLE_EQ(dot_product(a3, a4), 14.5);

    std::array<int, 0> emptyArr1 = {};
    std::array<int, 0> emptyArr2 = {};
    EXPECT_EQ(dot_product(emptyArr1, emptyArr2), 0);
}
