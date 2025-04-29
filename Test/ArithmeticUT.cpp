#include "TestCommon.hpp"

TEST(ArrayOperationsTest, Addition)
{
    auto a = enda::ones<double>(5);

    enda::basic_array<double, 1, C_layout, 'A', heap<>> b(5);
    for (int i = 0; i < 5; ++i)
    {
        b[i] = 2.0;
    }

    auto c = a + b;

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_NEAR(c[i], 3.0, 1e-10) << "Mismatch at index " << i;
    }
}

TEST(ArrayOperationsTest, Subtraction)
{
    auto a = enda::ones<double>(5);

    enda::basic_array<double, 1, C_layout, 'A', heap<>> b(5);
    for (int i = 0; i < 5; ++i)
    {
        b[i] = 0.5;
    }

    auto c = a - b;

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_NEAR(c[i], 0.5, 1e-10) << "Mismatch at index " << i;
    }
}

TEST(ArrayOperationsTest, Multiplication)
{
    auto a = enda::ones<double>(5);

    enda::basic_array<double, 1, C_layout, 'A', heap<>> b(5);
    for (int i = 0; i < 5; ++i)
    {
        b[i] = 3.0;
    }

    auto c = a * b;

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_NEAR(c[i], 3.0, 1e-10) << "Mismatch at index " << i;
    }
}

TEST(ArrayOperationsTest, Division)
{
    enda::basic_array<double, 1, C_layout, 'A', heap<>> a(5);
    for (int i = 0; i < 5; ++i)
    {
        a[i] = 4.0;
    }

    auto b = enda::ones<double>(5);

    auto c = a / b;

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_NEAR(c[i], 4.0, 1e-10) << "Mismatch at index " << i;
    }
}
