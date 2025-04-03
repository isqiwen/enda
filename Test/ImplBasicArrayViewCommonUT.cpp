#include "TestCommon.hpp"

// Test constructing a basic_array from dimensions.
TEST(BasicArrayTest, ConstructFromDimensions)
{
    // Create a 3x2 array of int using the constructor with exactly Rank arguments.
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr(3, 2);

    // Test that the shape of the array is correctly set.
    auto shape = arr.indexmap().lengths();
    EXPECT_EQ(shape[0], 3);
    EXPECT_EQ(shape[1], 2);

    // Test that the total size equals 3*2.
    EXPECT_EQ(arr.indexmap().size(), 6);
}

// Test zeros() static member function.
TEST(BasicArrayTest, ZerosArray)
{
    // Create a 4x5 zero-initialized array.
    auto arr = enda::basic_array<double, 2, C_layout, 'A', heap<>>::zeros({4, 5});

    // Verify that every element is 0.0.
    for (long i = 0; i < arr.indexmap().size(); ++i)
    {
        // Access via linear index using operator() with _linear_index_t (simulate as i)
        _linear_index_t linIdx {i};
        EXPECT_DOUBLE_EQ(arr(linIdx), 0.0);
    }
}

// Test ones() static member function.
TEST(BasicArrayTest, OnesArray)
{
    // Create a 2x3 one-initialized array.
    auto arr = enda::basic_array<int, 2, C_layout, 'A', heap<>>::ones({2, 3});

    // Verify that every element is 1.
    for (long i = 0; i < arr.indexmap().size(); ++i)
    {
        _linear_index_t linIdx {i};
        EXPECT_EQ(arr(linIdx), 1);
    }
}

// Test scalar assignment operator.
TEST(BasicArrayTest, ScalarAssignment)
{
    // Create a 1D array of size 5.
    enda::basic_array<int, 1, C_layout, 'A', heap<>> arr(5);

    // Assign scalar 42 to the entire array.
    arr = 42;

    // Verify that every element is 42.
    for (long i = 0; i < arr.indexmap().size(); ++i)
    {
        _linear_index_t linIdx {i};
        EXPECT_EQ(arr(linIdx), 42);
    }
}

// Test assignment operator from another array (deep copy).
TEST(BasicArrayTest, DeepCopyAssignment)
{
    // Create a 2D array with dimensions 3x2.
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr1(3, 2);
    // Fill arr1 with sequential values.
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 2; ++j)
            arr1(i, j) = i * 10 + j;

    // Create another array by deep-copying arr1.
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr2 = arr1;

    // Verify that arr2 has the same values as arr1.
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 2; ++j)
            EXPECT_EQ(arr2(i, j), arr1(i, j));
}

// Test resize() function.
TEST(BasicArrayTest, ResizeArray)
{
    // Create a 1D array of size 5.
    enda::basic_array<int, 1, C_layout, 'A', heap<>> arr(5);

    // Fill with a value.
    arr = 7;
    // Resize the array to size 10.
    arr.resize(10);

    // After resizing, the new size should be 10.
    EXPECT_EQ(arr.indexmap().size(), 10);
}

// Test conversion to a view using as_array_view().
TEST(BasicArrayTest, AsArrayViewConversion)
{
    // Create a 2D array with dimensions 3x3 and fill it with ones.
    auto arr = enda::basic_array<int, 2, C_layout, 'A', heap<>>::ones({3, 3});

    // Convert the array to a view.
    auto view = arr.as_array_view();

    // Verify that the view's shape matches.
    auto viewShape = view.indexmap().lengths();
    EXPECT_EQ(viewShape[0], 3);
    EXPECT_EQ(viewShape[1], 3);

    // Verify that all elements in the view are 1.
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            EXPECT_EQ(view(i, j), 1);
}

// Test constructing an array using initializer list (1D).
TEST(BasicArrayTest, InitializerList1D)
{
    // Construct a 1D array using an initializer list.
    enda::basic_array<int, 1, C_layout, 'A', heap<>> arr {{10, 20, 30, 40, 50}};

    // Verify that the size is 5.
    EXPECT_EQ(arr.indexmap().size(), 5);
    // Verify each element.
    int expected = 10;
    for (int i = 0; i < 5; ++i)
    {
        _linear_index_t linIdx {i};
        EXPECT_EQ(arr(linIdx), expected);
        expected += 10;
    }
}

// Test constructing an array using initializer list (2D).
TEST(BasicArrayTest, InitializerList2D)
{
    // Construct a 2D array using a nested initializer list.
    enda::basic_array<int, 2, C_layout, 'A', heap<>> arr({{1, 2, 3}, {4, 5, 6}});

    // Verify the shape is 2x3.
    auto shape = arr.indexmap().lengths();
    EXPECT_EQ(shape[0], 2);
    EXPECT_EQ(shape[1], 3);

    // Verify specific element: arr(1, 2) should be 6.
    EXPECT_EQ(arr(1, 2), 6);
}
