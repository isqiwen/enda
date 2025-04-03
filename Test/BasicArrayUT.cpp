#include "TestCommon.hpp"

// Test case for constructing a basic_array using dimensions.
TEST(BasicArrayTest, ConstructArrayWithDimensions)
{
    // Create a 3x2 array of int
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr(3, 2);

    // Check the shape
    auto shape = arr.indexmap().lengths();
    EXPECT_EQ(shape[0], 3);
    EXPECT_EQ(shape[1], 2);

    // Check the size (total number of elements)
    EXPECT_EQ(arr.indexmap().size(), 3 * 2);
}

// Test case for zeros: Create an array of zeros.
TEST(BasicArrayTest, ZerosArray)
{
    // Create a 4x5 zero-initialized array
    auto arr = enda::basic_array<double, 2, C_layout, 'A', heap<>>::zeros({4, 5});

    // Iterate over the array and check all elements are zero
    for (const auto& val : arr)
    {
        EXPECT_DOUBLE_EQ(val, 0.0);
    }
}

// Test case for ones: Create an array of ones.
TEST(BasicArrayTest, OnesArray)
{
    // Create a 2x3 one-initialized array
    auto arr = enda::basic_array<int, 2, C_layout, 'A', heap<>>::ones({2, 3});

    // Check that each element is 1.
    for (const auto& val : arr)
    {
        EXPECT_EQ(val, 1);
    }
}

// Test case for scalar assignment operator.
TEST(BasicArrayTest, ScalarAssignment)
{
    // Create a 1-dimensional array of size 5
    enda::basic_array<int, 1, C_layout, 'A', heap<>> arr(5);

    // Assign scalar 42 to the entire array
    arr = 42;

    // Verify each element is 42
    for (const auto& val : arr)
    {
        EXPECT_EQ(val, 42);
    }
}

// Test case for array view conversion.
TEST(BasicArrayTest, ArrayViewConversion)
{
    // Create a 3x3 array with ones using the ones factory function.
    auto arr = enda::basic_array<int, 2, C_layout, 'A', heap<>>::ones({3, 3});

    // Get an array view from the array.
    auto view = arr.as_array_view();

    // Verify that the view's shape matches and the elements are equal.
    auto shape = view.indexmap().lengths();
    EXPECT_EQ(shape[0], 3);
    EXPECT_EQ(shape[1], 3);

    for (size_t i = 0; i < 3; ++i)
        for (size_t j = 0; j < 3; ++j)
            EXPECT_EQ(view(i, j), 1);
}

// Test case for constructing an array from an initializer list (for 1D and 2D arrays).
TEST(BasicArrayTest, InitializerListConstruction)
{
    // 1D array initializer list
    enda::basic_array<int, 1, C_layout, 'A', heap<>> arr1 {{1, 2, 3, 4, 5}};
    EXPECT_EQ(arr1.indexmap().size(), 5);
    int idx = 0;
    for (const auto& v : arr1)
    {
        EXPECT_EQ(v, idx + 1);
        ++idx;
    }

    // 2D array initializer list
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr2 {{{1, 2, 3}, {4, 5, 6}}};
    EXPECT_EQ(arr2.indexmap().size(), 6);
    // Check specific element: element at (1,2) should be 6.
    EXPECT_EQ(arr2(1, 2), 6);
}

TEST(BasicArrayTest, Zeros)
{
    auto a = enda::zeros<long>(3, 3);

    EXPECT_EQ(a.shape(), shape_t<2>({3, 3}));
    EXPECT_EQ(max_element(abs(a)), 0);
}

TEST(BasicArrayTest, ZeroStaticFactory)
{
    auto a1 = enda::array<long, 1>::zeros({3});
    auto a2 = enda::array<long, 2>::zeros({3, 4});
    auto a3 = enda::array<long, 3>::zeros({3, 4, 5});

    EXPECT_EQ(a1.shape(), (enda::shape_t<1> {3}));
    EXPECT_EQ(a2.shape(), (enda::shape_t<2> {3, 4}));
    EXPECT_EQ(a3.shape(), (enda::shape_t<3> {3, 4, 5}));

    EXPECT_EQ(max_element(abs(a1)), 0);
    EXPECT_EQ(max_element(abs(a2)), 0);
    EXPECT_EQ(max_element(abs(a3)), 0);
}

struct Int
{
    int i = 2;
};

TEST(BasicArrayTest, ZerosCustom)
{
    auto a = enda::zeros<Int>(3, 3);

    EXPECT_EQ(a.shape(), (enda::shape_t<2> {3, 3}));
    for (auto v : a)
    {
        EXPECT_EQ(v.i, 0);
    }
}

TEST(BasicArrayTest, ChangeData)
{
    enda::array<long, 3> a(3, 3, 4);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
                a(i, j, k) = i + 10 * j + 100 * k;

    auto v = a(_, 1, 2);

    EXPECT_EQ((enda::slice_static::detail::slice_layout_prop(
                  1, true, std::array<bool, 3> {1, 0, 0}, std::array<int, 3> {0, 1, 2}, enda::layout_prop_e::contiguous, 128, 0)),
              enda::layout_prop_e::strided_1d);

    EXPECT_EQ(v.shape(), (enda::shape_t<1> {3}));

    EXPECT_EQ(a(1, 1, 2), 1 + 10 * 1 + 100 * 2);

    a(1, 1, 2) = -28;
    EXPECT_EQ(v(1), a(1, 1, 2));
}

TEST(BasicArrayTest, OnRawPointers)
{
    std::vector<long> _data(10, 3);

    enda::array_view<long const, 2> a({3, 3}, _data.data());

    EXPECT_EQ(a(1, 1), 3);
}

TEST(BasicArrayTest, add)
{
    std::vector<long> v1(10), v2(10), vr(10, -1);
    for (int i = 0; i < 10; ++i)
    {
        v1[i] = i;
        v2[i] = 10l * i;
    }

    enda::array_view<long const, 2> a({3, 3}, v1.data());
    enda::array_view<long const, 2> b({3, 3}, v2.data());
    enda::array_view<long, 2>       c({3, 3}, vr.data());

    c = a + b;

    for (int i = 0; i < 9; ++i)
        EXPECT_EQ(vr[i], 11 * i);
    EXPECT_EQ(vr[9], -1);
}
