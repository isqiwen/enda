#include <gtest/gtest.h>

#include "BasicArray.hpp"

using namespace enda;

// Test constructing a 1D basic_array_view from a pointer.
TEST(BasicArrayViewTest, ConstructFromPointer)
{
    // Create a simple 1D array of ints.
    int                 data[] = {10, 20, 30, 40, 50};
    std::array<long, 1> shape  = {5};

    // Construct a basic_array_view from a pointer and shape.
    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view(shape, data);

    // Verify the shape using the layout's lengths.
    auto viewShape = view.indexmap().lengths();
    EXPECT_EQ(viewShape[0], 5);

    // Verify element access.
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(view(i), data[i]);
    }
}

// Test constructing a 1D basic_array_view from a std::array.
TEST(BasicArrayViewTest, ConstructFromStdArray)
{
    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    // Deduction guide creates a 1D view from a std::array.
    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view(arr);

    // Verify the shape.
    auto viewShape = view.indexmap().lengths();
    EXPECT_EQ(viewShape[0], 5);

    // Verify element access.
    for (size_t i = 0; i < arr.size(); i++)
    {
        EXPECT_EQ(view(i), arr[i]);
    }
}

// Test constructing a 2D basic_array_view from a pointer.
TEST(BasicArrayViewTest, Construct2DViewFromPointer)
{
    // Create a 2D array (3 rows x 4 columns) in row-major order.
    int                 data[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<long, 2> shape    = {3, 4};

    // Construct a 2D basic_array_view from a pointer and shape.
    enda::basic_array_view<int, 2, C_layout, 'A', default_accessor, borrowed<>> view(shape, data);

    // Verify the shape.
    auto viewShape = view.indexmap().lengths();
    EXPECT_EQ(viewShape[0], 3);
    EXPECT_EQ(viewShape[1], 4);

    // Verify element access using the view's function call operator.
    EXPECT_EQ(view(0, 0), 1);
    EXPECT_EQ(view(1, 2), 7);
    EXPECT_EQ(view(2, 3), 12);
}

// Test the swap functionality of basic_array_view.
TEST(BasicArrayViewTest, SwapViews)
{
    // Create two 1D views from two different arrays.
    int                 data1[] = {10, 20, 30};
    int                 data2[] = {40, 50, 60};
    std::array<long, 1> shape   = {3};

    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view1(shape, data1);
    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view2(shape, data2);

    // Verify initial elements.
    EXPECT_EQ(view1(0), 10);
    EXPECT_EQ(view2(0), 40);

    // Swap the two views using the friend swap.
    using std::swap;
    swap(view1, view2);

    // After swap, view1 should now point to data2 and view2 to data1.
    EXPECT_EQ(view1(0), 40);
    EXPECT_EQ(view2(0), 10);
}

// Test the rebind functionality of basic_array_view.
TEST(BasicArrayViewTest, RebindViews)
{
    // Create two 1D views.
    int                 data1[] = {1, 2, 3, 4};
    int                 data2[] = {5, 6, 7, 8};
    std::array<long, 1> shape   = {4};

    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view1(shape, data1);
    enda::basic_array_view<int, 1, C_layout, 'V', default_accessor, borrowed<>> view2(shape, data2);

    // Rebind view1 to view2.
    view1.rebind(view2);

    // After rebinding, view1 should mirror view2.
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(view1(i), view2(i));
    }
}

// Test assignment operator from a scalar.
TEST(BasicArrayViewTest, ScalarAssignment)
{
    // Create a 1D array view from a pointer.
    int                 data[] = {1, 2, 3, 4, 5};
    std::array<long, 1> shape  = {5};

    // Construct a non-const view (for assignment, value_type must not be const).
    enda::basic_array_view<int, 1, C_layout, 'A', default_accessor, borrowed<>> view(shape, data);

    // Assign a scalar value (e.g., 42) to all elements.
    view = 42;

    // Verify all elements are assigned 42.
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(view(i), 42);
    }
}
