#include "TestCommon.hpp"

// Test zeros for a scalar (Rank==0)
TEST(BasicFunctionsTest, ZerosScalar)
{
    // For Rank == 0, zeros should return a default constructed T (0 for int)
    int s = zeros<int>(std::array<long, 0> {});
    EXPECT_EQ(s, 0);
}

// Test zeros for a 1-dimensional array
TEST(BasicFunctionsTest, ZerosArray1D)
{
    auto a = zeros<int>(std::array<long, 1> {5});
    // Assuming that 'a' provides a size() method and operator()(index) for access.
    EXPECT_EQ(a.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(a(i), 0);
    }
}

// Test ones for a scalar (Rank==0)
TEST(BasicFunctionsTest, OnesScalar)
{
    int s = ones<int>(std::array<long, 0> {});
    EXPECT_EQ(s, 1);
}

// Test ones for a 1-dimensional array
TEST(BasicFunctionsTest, OnesArray1D)
{
    auto a = ones<int>(std::array<long, 1> {5});
    EXPECT_EQ(a.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(a(i), 1);
    }
}

// Test arange function with explicit start, end and step
TEST(BasicFunctionsTest, ArangeFunction)
{
    auto a = arange(0, 5, 1);
    EXPECT_EQ(a.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(a(i), i);
    }

    // Test arange with only end parameter (start defaults to 0)
    auto a2 = arange(5);
    EXPECT_EQ(a2.size(), 5);
    for (int i = 0; i < 5; i++)
    {
        EXPECT_EQ(a2(i), i);
    }
}

// Test random number generation for a scalar
TEST(BasicFunctionsTest, RandScalar)
{
    double r = rand<>(std::array<long, 0> {});
    EXPECT_GE(r, 0.0);
    EXPECT_LT(r, 1.0);
}

// Test random number generation for a 1-dimensional array
TEST(BasicFunctionsTest, RandArray1D)
{
    auto a = rand<>(std::array<long, 1> {10});
    EXPECT_EQ(a.size(), 10);
    for (int i = 0; i < 10; i++)
    {
        double val = a(i);
        EXPECT_GE(val, 0.0);
        EXPECT_LT(val, 1.0);
    }
}

// Test first_dim and second_dim for a 2-dimensional array
TEST(BasicFunctionsTest, ArrayDimensions)
{
    auto a = ones<int>(std::array<long, 2> {3, 4});
    EXPECT_EQ(first_dim(a), 3);
    EXPECT_EQ(second_dim(a), 4);
}

// Test operator== for comparing an array with a contiguous range (std::vector)
TEST(BasicFunctionsTest, ArrayEquality)
{
    auto             a   = arange(5);
    std::vector<int> vec = {0, 1, 2, 3, 4};
    EXPECT_TRUE(a == vec);

    auto b = a; // Copy construction
    EXPECT_TRUE(a == b);
}

// Test get_block_layout for a contiguous 2D array.
// For a contiguous array, we expect one block with block size equal to the total number of elements.
TEST(BasicFunctionsTest, GetBlockLayout)
{
    auto a          = ones<int>(std::array<long, 2> {2, 3});
    auto layout_opt = get_block_layout(a);
    ASSERT_TRUE(layout_opt.has_value());
    auto [n_blocks, block_size, block_str] = layout_opt.value();
    // For a contiguous array (assumed row-major), usually strides are {3, 1}.
    EXPECT_EQ(n_blocks, 1);
    EXPECT_EQ(block_size, a.size()); // 2*3 = 6
    // For block_str, here we assume it equals the total number of elements in the first dimension times elements per row.
    EXPECT_EQ(block_str, first_dim(a) * (a.size() / first_dim(a)));
}

// Test concatenation along axis 0 for 2D arrays
TEST(BasicFunctionsTest, ConcatenateAxis0)
{
    auto                a0             = ones<int>(std::array<long, 2> {2, 3});
    auto                a1             = zeros<int>(std::array<long, 2> {2, 3});
    auto                cat            = concatenate<0>(a0, a1);
    std::array<long, 2> expected_shape = {4, 3};
    EXPECT_EQ(cat.shape(), expected_shape);
    // Check that the first two rows are all 1s and the last two rows are all 0s
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            EXPECT_EQ(cat(i, j), 1);
        }
    }
    for (int i = 2; i < 4; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            EXPECT_EQ(cat(i, j), 0);
        }
    }
}

// Test concatenation along axis 1 for 2D arrays
TEST(BasicFunctionsTest, ConcatenateAxis1)
{
    auto                a0             = ones<int>(std::array<long, 2> {2, 3});
    auto                a1             = zeros<int>(std::array<long, 2> {2, 3});
    auto                cat            = concatenate<1>(a0, a1);
    std::array<long, 2> expected_shape = {2, 6};
    EXPECT_EQ(cat.shape(), expected_shape);
    // Check that in each row, the first 3 elements are 1s and the last 3 elements are 0s.
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            EXPECT_EQ(cat(i, j), 1);
        }
        for (int j = 3; j < 6; j++)
        {
            EXPECT_EQ(cat(i, j), 0);
        }
    }
}

// Test view related functions: make_const_view, make_array_view, make_array_const_view, and make_matrix_view.
TEST(BasicFunctionsTest, MakeViews)
{
    auto a                = ones<int>(std::array<long, 2> {2, 2});
    auto const_view       = make_const_view(a);
    auto array_view       = make_array_view(a);
    auto array_const_view = make_array_const_view(a);
    auto matrix_view      = make_matrix_view(a);
    // Assuming that these views compare equal to the original array.
    EXPECT_TRUE(a == const_view);
    EXPECT_TRUE(a == array_view);
    EXPECT_TRUE(a == array_const_view);
    EXPECT_TRUE(a == matrix_view);
}

// Test make_regular (for non-regular types, it should return a basic_array; otherwise, forward the object)
TEST(BasicFunctionsTest, MakeRegular)
{
    auto a   = ones<int>(std::array<long, 1> {5});
    auto reg = make_regular(a);
    EXPECT_TRUE(a == reg);
}

// Test resize_or_check_if_view: For a regular array with the correct shape it should do nothing,
// and for a view with a mismatched shape it should throw an exception.
TEST(BasicFunctionsTest, ResizeOrCheckIfView)
{
    auto                a             = ones<int>(std::array<long, 2> {2, 3});
    std::array<long, 2> correct_shape = {2, 3};
    EXPECT_NO_THROW(resize_or_check_if_view(a, correct_shape));

    // For a view, if the shape does not match, it should throw an exception.
    auto                a_view      = basic_array_view(a);
    std::array<long, 2> wrong_shape = {3, 3};
    EXPECT_THROW(resize_or_check_if_view(a_view, wrong_shape), enda::runtime_error);
}
