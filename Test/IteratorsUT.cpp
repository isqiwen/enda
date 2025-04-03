#include "TestCommon.hpp"

// ===================   for_each ===========================================

TEST(for_each, Mutable)
{
    enda::array<int, 3> a(3, 4, 5);
    enda::for_each(a.shape(), [&a, c2 = 0](auto... i) mutable { a(i...) = c2++; });

    auto check = a;
    int  c     = 0;
    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
            for (int k = 0; k < a.extent(2); ++k)
                check(i, j, k) = c++;

    EXPECT_ARRAY_EQ(a, check);
}

// ====================   iterator ==========================================

TEST(IteratorTest, empty)
{
    enda::array<int, 1> arr(0);
    int                 s = 0;
    for (auto i : arr)
        s += i;
    EXPECT_EQ(s, 0);
}

//-----------------------------

TEST(IteratorTest, Contiguous1d)
{
    enda::array<long, 1> a;
    for (int i = 0; i < a.extent(0); ++i)
        a(i) = 1 + i;

    long c = 1;
    for (auto x : a)
    {
        EXPECT_EQ(x, c++);
    }
}

//-----------------------------

TEST(IteratorTest, Contiguous2d)
{
    enda::array<long, 2> a(2, 3);

    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
            a(i, j) = 1 + i + 10 * j;

    auto it = a.begin();

    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
        {
            EXPECT_EQ(*it, a(i, j));
            EXPECT_FALSE(it == a.end());
            ++it;
        }
}

//-----------------------------

TEST(IteratorTest, Contiguous3d)
{
    enda::array<long, 3> a(3, 5, 9);

    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
            for (int k = 0; k < a.extent(2); ++k)
                a(i, j, k) = 1 + i + 10 * j + 100 * k;

    auto it = a.begin();

    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
            for (int k = 0; k < a.extent(2); ++k)
            {
                EXPECT_EQ(*it, a(i, j, k));
                EXPECT_TRUE(it != a.end());
                ++it;
            }
}

//-----------------------------

TEST(IteratorTest, Strided3d)
{
    enda::array<long, 3> a(3, 5, 9);

    for (int i = 0; i < a.extent(0); ++i)
        for (int j = 0; j < a.extent(1); ++j)
            for (int k = 0; k < a.extent(2); ++k)
                a(i, j, k) = 1 + i + 10 * j + 100 * k;

    auto v = a(range(0, a.extent(0), 2), range(0, a.extent(1), 2), range(0, a.extent(2), 2));

    auto it = v.begin();

    for (int i = 0; i < v.extent(0); ++i)
        for (int j = 0; j < v.extent(1); ++j)
            for (int k = 0; k < v.extent(2); ++k)
            {
                EXPECT_EQ(*it, v(i, j, k));
                EXPECT_TRUE(it != v.end());
                ++it;
            }
    EXPECT_TRUE(it == v.end());
}

//-----------------------------

TEST(IteratorTest, BlockStrided2d)
{
    auto a = enda::rand<>(3, 4, 5);

    EXPECT_TRUE(get_block_layout(a(_, range(2), _)));
    EXPECT_TRUE(get_block_layout(a(_, range(3), _)));
    EXPECT_TRUE(get_block_layout(a(_, range(0, 4, 2), _)));
    EXPECT_TRUE(!get_block_layout(a(_, range(0, 4, 3), _)));

    auto av                                = a(_, range(0, 4, 2), _);
    auto [n_blocks, block_size, block_str] = get_block_layout(av).value();
    EXPECT_EQ(n_blocks * block_size, av.size());

    // Compare loop over array with pointer arithmetic based on block_size and block_str
    auto* ptr = a.data();
    for (auto [n, val] : enda::itertools::enumerate(av))
    {
        auto [bl_idx, inner_idx] = std::ldiv(n, block_size);
        EXPECT_EQ(val, *(ptr + bl_idx * block_str + inner_idx));
    }
}

//-----------------------------

TEST(IteratorTest, bug)
{
    const int              N1 = 1000, N2 = 1000;
    enda::array<double, 2> a(2 * N1, 2 * N2);
    auto                   v = a(range(0, -1, 2), range(0, -1, 2));
    for (auto& x : v)
    {
        x = 10;
    }
}
