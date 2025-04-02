#include <gtest/gtest.h>

#include "Enda.hpp"

using namespace enda;

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
