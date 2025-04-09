#include "TestCommon.hpp"

TEST(StackArray, create)
{
    enda::stack_array<long, 3, 3> a;
    enda::array<long, 2>          d(3, 3);

    a = 3;
    d = 3;
    EXPECT_ARRAY_NEAR(a, d);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            a(i, j) = i + 10 * j;
            d(i, j) = i + 10 * j;
        }

    auto ac = a;

    ac = a + d;

    ENDA_PRINT(a.indexmap());
    // ENDA_PRINT(ac);
    ENDA_PRINT(ac.indexmap());

    EXPECT_ARRAY_NEAR(a, d);
}

// ==============================================================

TEST(StackArray, slice)
{
    enda::stack_array<long, 3, 3> a;

    a = 3;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            a(i, j) = i + 10 * j;
        }

    {
        auto v = a(_, 1);

        enda::array<long, 2> ad {a};
        enda::array<long, 1> vd {v};

        ENDA_PRINT(v.indexmap());
        ENDA_PRINT(a);
        ENDA_PRINT(v);

        EXPECT_ARRAY_NEAR(a, ad);
        EXPECT_ARRAY_NEAR(v, vd);
    }

    {
        auto v = a(1, _);

        enda::array<long, 2> ad {a};
        enda::array<long, 1> vd {v};

        ENDA_PRINT(v.indexmap());
        ENDA_PRINT(a);
        ENDA_PRINT(v);

        EXPECT_ARRAY_NEAR(a, ad);
        EXPECT_ARRAY_NEAR(v, vd);
    }
}

// ==============================================================

TEST(Loop, Static)
{
    enda::array<long, 2> a(3, 3);

    enda::for_each_static<enda::encode(std::array {3, 3}), 0>(a.shape(), [&a](auto x0, auto x1) { a(x0, x1) = x0 + 10 * x1; });

    std::cout << a << std::endl;
}
