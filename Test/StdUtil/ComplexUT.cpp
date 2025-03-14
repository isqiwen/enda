#include <gtest/gtest.h>

#include "StdUtil/Complex.hpp"

TEST(ComplexOperatorTest, Addition)
{
    std::complex<double> c1(1.0, 2.0);
    double               d      = 3.0;
    auto                 result = c1 + d;
    EXPECT_DOUBLE_EQ(result.real(), 4.0);
    EXPECT_DOUBLE_EQ(result.imag(), 2.0);
}

TEST(ComplexOperatorTest, Subtraction)
{
    std::complex<float> c1(5.0f, 2.0f);
    int                 d      = 2;
    auto                result = c1 - d;
    EXPECT_FLOAT_EQ(result.real(), 3.0f);
    EXPECT_FLOAT_EQ(result.imag(), 2.0f);
}

TEST(ComplexOperatorTest, Multiplication)
{
    std::complex<int> c1(2, 3);
    float             d      = 1.5f;
    auto              result = c1 * d;
    EXPECT_FLOAT_EQ(result.real(), 3.0f);
    EXPECT_FLOAT_EQ(result.imag(), 4.5f);
}

TEST(ComplexOperatorTest, Division)
{
    std::complex<double> c1(4.0, 2.0);
    double               d      = 2.0;
    auto                 result = c1 / d;
    EXPECT_DOUBLE_EQ(result.real(), 2.0);
    EXPECT_DOUBLE_EQ(result.imag(), 1.0);
}

TEST(ComplexOperatorTest, ComplexToComplexAddition)
{
    std::complex<float>  c1(1.0f, 2.0f);
    std::complex<double> c2(3.0, 4.0);
    auto                 result = c1 + c2;
    EXPECT_DOUBLE_EQ(result.real(), 4.0);
    EXPECT_DOUBLE_EQ(result.imag(), 6.0);
}

TEST(ComplexOperatorTest, ComplexToComplexMultiplication)
{
    std::complex<int>    c1(1, 2);
    std::complex<double> c2(2.0, 3.0);
    auto                 result = c1 * c2;
    EXPECT_DOUBLE_EQ(result.real(), -4.0);
    EXPECT_DOUBLE_EQ(result.imag(), 7.0);
}
