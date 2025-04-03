#include "TestCommon.hpp"

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

TEST(BasicArrayViewTest, ViewBasic)
{
    enda::array<long, 3> a(3, 3, 4);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
                a(i, j, k) = i + 10 * j + 100 * k;

    auto v = a(_, 1, 2);

    EXPECT_EQ(v.shape(), (enda::shape_t<1> {3}));

    EXPECT_EQ(a(1, 1, 2), 1 + 10 * 1 + 100 * 2);

    a(1, 1, 2) = -28;
    EXPECT_EQ(v(1), a(1, 1, 2));
}
// ----------------------------------

TEST(BasicArrayViewTest, Ellipsis)
{ // NOLINT
    enda::array<long, 3> A(2, 3, 4);
    A() = 7;

    EXPECT_ARRAY_NEAR(A(0, ___), A(0, _, _), 1.e-15);

    enda::array<long, 4> B(2, 3, 4, 5);
    B() = 8;

    EXPECT_ARRAY_NEAR(B(0, ___, 3), B(0, _, _, 3), 1.e-15);
    EXPECT_ARRAY_NEAR(B(0, ___, 2, 3), B(0, _, 2, 3), 1.e-15);
    EXPECT_ARRAY_NEAR(B(___, 2, 3), B(_, _, 2, 3), 1.e-15);
}

// ----------------------------------

template<typename ArrayType>
auto sum0(ArrayType const& A)
{
    enda::array<typename ArrayType::value_type, ArrayType::rank - 1> res = A(0, ___);
    for (size_t u = 1; u < A.shape()[0]; ++u)
        res += A(u, ___);
    return res;
}

TEST(BasicArrayViewTest, Ellipsis2)
{
    enda::array<double, 2> A(5, 2);
    A() = 2;
    enda::array<double, 3> B(5, 2, 3);
    B() = 3;
    EXPECT_ARRAY_NEAR(sum0(A), enda::array<double, 1> {10, 10}, 1.e-15);
    EXPECT_ARRAY_NEAR(sum0(B), enda::array<double, 2> {{15, 15, 15}, {15, 15, 15}}, 1.e-15);
}

// ==============================================================

TEST(BasicArrayViewTest, ConstView)
{
    enda::array<long, 2> A(2, 3);
    A() = 98;

    auto f2 = [](enda::array_view<long, 2> const&) {};

    f2(A());

    enda::array_const_view<long, 2> {A()};
}

// ==============================================================

TEST(BasicArrayViewTest, Bug2)
{
    enda::array<double, 3> A(10, 2, 2);
    A() = 0;

    A(4, range::all, range::all) = 1;
    A(5, range::all, range::all) = 2;

    matrix_view<double> M1 = A(4, range::all, range::all);
    matrix_view<double> M2 = A(5, range::all, range::all);

    EXPECT_ARRAY_NEAR(M1, matrix<double> {{1, 1}, {1, 1}});
    EXPECT_ARRAY_NEAR(M2, matrix<double> {{2, 2}, {2, 2}});

    M1 = M2;

    EXPECT_ARRAY_NEAR(M1, matrix<double> {{2, 2}, {2, 2}});
    EXPECT_ARRAY_NEAR(M2, matrix<double> {{2, 2}, {2, 2}});
}

// ==============================================================

TEST(BasicArrayViewTest, View)
{
    enda::array<long, 2> A(2, 3);
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            A(i, j) = 10 * i + j;

    enda::array_view<long, 2> AA(A);

    std::stringstream fs1, fs2;
    fs1 << A;
    fs2 << AA;
    EXPECT_EQ(fs1.str(), fs2.str());
    EXPECT_EQ(AA(0, 0), 0);

    enda::array_view<long, 1> SL1(A(0, range(0, 3)));
    enda::array_view<long, 1> SL2(A(1, range(0, 2)));
    enda::array_view<long, 1> SL3(A(1, range(1, 3)));
    enda::array_view<long, 1> SL4(A(range(0, 2), 0));
    enda::array_view<long, 1> SL5(A(range(0, 2), 1));

    EXPECT_EQ_ARRAY(SL1, (enda::array<long, 1> {0, 1, 2}));
    EXPECT_EQ_ARRAY(SL2, (enda::array<long, 1> {10, 11}));
    EXPECT_EQ_ARRAY(SL3, (enda::array<long, 1> {11, 12}));
    EXPECT_EQ_ARRAY(SL4, (enda::array<long, 1> {0, 10}));
    EXPECT_EQ_ARRAY(SL5, (enda::array<long, 1> {1, 11}));
}

// ==============================================================

TEST(BasicArrayViewTest, View3)
{
    using enda::encode;
    //-------------

    enda::array<long, 3>                                                                                       A0(2, 3, 4);
    enda::array<long, 3, F_layout>                                                                             Af(2, 3, 4);
    enda::array<long, 3, enda::basic_layout<0, encode(std::array {0, 1, 2}), enda::layout_prop_e::contiguous>> Ac(2, 3, 4);
    enda::array<long, 3, enda::basic_layout<0, encode(std::array {2, 1, 0}), enda::layout_prop_e::contiguous>> A1(2, 3, 4);

    // non trivial permutation
    enda::array<long, 3, enda::basic_layout<0, encode(std::array {2, 0, 1}), enda::layout_prop_e::contiguous>> A2(2, 3, 4);
    enda::array<long, 3, enda::basic_layout<0, encode(std::array {1, 2, 0}), enda::layout_prop_e::contiguous>> A3(2, 3, 4);

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
            {
                EXPECT_EQ((A0.indexmap()(i, j, k)), (3 * 4) * i + 4 * j + k);
                EXPECT_EQ((A1.indexmap()(i, j, k)), i + 2 * j + (2 * 3) * k);
                EXPECT_EQ((Ac.indexmap()(i, j, k)), (3 * 4) * i + 4 * j + k);
                EXPECT_EQ((Af.indexmap()(i, j, k)), i + 2 * j + (2 * 3) * k);

                EXPECT_EQ((A2.indexmap()(i, j, k)), 3 * i + j + (2 * 3) * k);
                EXPECT_EQ((A3.indexmap()(i, j, k)), i + (2 * 4) * j + 2 * k);
            }

    //-------------
    auto f = [](auto& A) {
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 4; ++k)
                    A(i, j, k) = 100 * (i + 1) + 10 * (j + 1) + (k + 1);

        auto _ = range::all;

        EXPECT_EQ_ARRAY(A(0, _, _), (enda::array<long, 2> {{111, 112, 113, 114}, {121, 122, 123, 124}, {131, 132, 133, 134}}));
        EXPECT_EQ_ARRAY(A(1, _, _), (enda::array<long, 2> {{211, 212, 213, 214}, {221, 222, 223, 224}, {231, 232, 233, 234}}));
        EXPECT_EQ_ARRAY(A(_, 0, _), (enda::array<long, 2> {{111, 112, 113, 114}, {211, 212, 213, 214}}));
        EXPECT_EQ_ARRAY(A(_, _, 1), (enda::array<long, 2> {{112, 122, 132}, {212, 222, 232}}));
        EXPECT_EQ_ARRAY(A(_, 0, 1), (enda::array<long, 1> {112, 212}));
    };

    f(A0);
    f(A1);
    f(A2);
    f(A3);
    f(Ac);
    f(Af);
}

// ==============================================================

// old issue
TEST(BasicArrayViewTest, IssueXXX)
{
    enda::array<double, 3> A(10, 2, 2);
    A() = 0;

    A(4, range::all, range::all) = 1;
    A(5, range::all, range::all) = 2;

    matrix_view<double> M1 = A(4, range::all, range::all);
    matrix_view<double> M2 = A(5, range::all, range::all);

    EXPECT_ARRAY_NEAR(M1, matrix<double> {{1, 1}, {1, 1}});
    EXPECT_ARRAY_NEAR(M2, matrix<double> {{2, 2}, {2, 2}});

    M1 = M2;

    EXPECT_ARRAY_NEAR(M1, matrix<double> {{2, 2}, {2, 2}});
    EXPECT_ARRAY_NEAR(M2, matrix<double> {{2, 2}, {2, 2}});
}
