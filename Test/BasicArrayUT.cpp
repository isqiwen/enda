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

// ==============================================================

TEST(BasicArrayTest, Create1)
{
    auto A = enda::array<long, 2>(3, 3);
    EXPECT_EQ(A.shape(), (enda::shape_t<2> {3, 3}));

    auto B = enda::array<long, 2>(std::array {3, 3});
    EXPECT_EQ(B.shape(), (enda::shape_t<2> {3, 3}));

    auto C = enda::array<long, 1>(3, 3);
    EXPECT_EQ(C.shape(), (enda::shape_t<1> {3}));
    EXPECT_EQ(C, (3 * enda::ones<long>(3)));
}

// -------------------------------------

TEST(BasicArrayTest, Contiguous)
{
    enda::array<long, 2> A {{1, 2}, {3, 4}, {5, 6}};
    enda::array<long, 2> B;
    B = A;

    EXPECT_ARRAY_NEAR(A, B);

    A(0, 1) = 87;
    B       = A(); // no resize

    EXPECT_EQ(A.indexmap().strides(), B.indexmap().strides());

    EXPECT_ARRAY_NEAR(A, B);
    EXPECT_EQ(B.shape(), (enda::shape_t<2> {3, 2}));
}

// -------------------------------------

TEST(BasicArrayTest, Strided)
{
    enda::array<long, 3> A(3, 5, 9);
    enda::array<long, 1> B;

    for (int i = 0; i < A.extent(0); ++i)
        for (int j = 0; j < A.extent(1); ++j)
            for (int k = 0; k < A.extent(2); ++k)
                A(i, j, k) = 1 + i + 10 * j + 100 * k;

    ENDA_PRINT(A.shape());

    B = A(_, 0, 1);
    for (int i = 0; i < A.extent(0); ++i)
        EXPECT_EQ(A(i, 0, 1), B(i));

    B = A(1, _, 2);
    for (int i = 0; i < A.extent(1); ++i)
        EXPECT_EQ(A(1, i, 2), B(i));

    B = A(1, 3, _);

    for (int i = 0; i < A.extent(2); ++i)
        EXPECT_EQ(A(1, 3, i), B(i));

    // P =0 to force out of the first test of slice_layout_prop. We want to test the real algorithm
    EXPECT_EQ(enda::slice_static::detail::slice_layout_prop(
                  0, true, std::array<bool, 3> {1, 0, 0}, std::array<int, 3> {0, 1, 2}, enda::layout_prop_e::contiguous, 128, 0),
              enda::layout_prop_e::strided_1d);
    EXPECT_EQ(enda::slice_static::detail::slice_layout_prop(
                  0, true, std::array<bool, 3> {0, 1, 0}, std::array<int, 3> {0, 1, 2}, enda::layout_prop_e::contiguous, 128, 0),
              enda::layout_prop_e::strided_1d);
    EXPECT_EQ(enda::slice_static::detail::slice_layout_prop(
                  0, true, std::array<bool, 3> {0, 0, 1}, std::array<int, 3> {0, 1, 2}, enda::layout_prop_e::contiguous, 128, 0),
              enda::layout_prop_e::contiguous);

    static_assert(enda::get_layout_info<decltype(A(1, 3, _))>.prop == enda::layout_prop_e::contiguous);
    static_assert(enda::get_layout_info<decltype(A(1, _, 2))>.prop == enda::layout_prop_e::strided_1d);
    static_assert(enda::get_layout_info<decltype(A(_, 0, 1))>.prop == enda::layout_prop_e::strided_1d);
}

// -------------------------------------

TEST(BasicArrayTest, Strided2)
{
    enda::array<long, 3> A(3, 5, 9);
    enda::array<long, 1> B;

    for (int i = 0; i < A.extent(0); ++i)
        for (int j = 0; j < A.extent(1); ++j)
            for (int k = 0; k < A.extent(2); ++k)
                A(i, j, k) = 1 + i + 10 * j + 100 * k;

    B.resize(20);
    B = 0;

    static_assert(enda::get_layout_info<decltype(B(range(0, 2 * A.extent(0), 2)))>.prop == enda::layout_prop_e::strided_1d);

    B(range(0, 2 * A.extent(0), 2)) = A(_, 0, 1);
    ENDA_PRINT(B);
    ENDA_PRINT(A(_, 0, 1));

    for (int i = 0; i < A.extent(0); ++i)
        EXPECT_EQ(A(i, 0, 1), B(2 * i));

    B(range(0, 2 * A.extent(1), 2)) = A(1, _, 2);
    ENDA_PRINT(B);
    ENDA_PRINT(A(1, _, 2));

    for (int i = 0; i < A.extent(1); ++i)
        EXPECT_EQ(A(1, i, 2), B(2 * i));

    B(range(0, 2 * A.extent(2), 2)) = A(1, 3, _);
    ENDA_PRINT(B);
    ENDA_PRINT(A(1, 3, _));

    for (int i = 0; i < A.extent(2); ++i)
        EXPECT_EQ(A(1, 3, i), B(2 * i));
}

// -------------------------------------

TEST(BasicArrayTest, Iterator1)
{
    enda::array<long, 2> A {{0, 1, 2}, {3, 4, 5}};

    int i = 0;
    for (auto x : A)
        EXPECT_EQ(x, i++);
}

// -------------------------------------

TEST(BasicArrayTest, CreateResize)
{

    enda::array<long, 2> A;
    A.resize({3, 3});
    EXPECT_EQ(A.shape(), (enda::shape_t<2> {3, 3}));

    A.resize({4, 4});
    EXPECT_EQ(A.shape(), (enda::shape_t<2> {4, 4}));

    enda::array<double, 2> M;
    M.resize(3, 3);

    EXPECT_EQ(M.shape(), (enda::shape_t<2> {3, 3}));

    enda::array<long, 1> V;
    V.resize(10);

    EXPECT_EQ(V.shape(), (enda::shape_t<1> {10}));
}

// ==============================================================

TEST(BasicArrayTest, InitList)
{

    // 1d
    enda::array<double, 1> A = {1, 2, 3, 4};

    EXPECT_EQ(A.shape(), (enda::shape_t<1> {4}));

    for (int i = 0; i < 4; ++i)
        EXPECT_EQ(A(i), i + 1);

    // 2d
    enda::array<double, 2> B = {{1, 2}, {3, 4}, {5, 6}};

    EXPECT_EQ(B.shape(), (enda::shape_t<2> {3, 2}));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 2; ++j)
            EXPECT_EQ(B(i, j), j + 2 * i + 1);

    // 3d
    enda::array<double, 3> C = {{{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}}, {{100, 200, 300, 400}, {500, 600, 700, 800}, {900, 1000, 1100, 1200}}};

    EXPECT_EQ(C.shape(), (enda::shape_t<3> {2, 3, 4}));
    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
                EXPECT_EQ(C(i, j, k), (i == 0 ? 1 : 100) * (k + 4 * j + 1));

    // matrix
    enda::matrix<double> M = {{1, 2}, {3, 4}, {5, 6}};
    EXPECT_EQ(M.shape(), (enda::shape_t<2> {3, 2}));

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 2; ++j)
            EXPECT_EQ(M(i, j), j + 2 * i + 1);
}

// ==============================================================

TEST(BasicArrayTest, InitList2)
{

    // testing more complex cases
    enda::array<std::array<double, 2>, 1> aa {{0.0, 0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};
    EXPECT_EQ(aa(3), (std::array<double, 2> {1, 1}));

    enda::array<double, 1> a {1, 2, 3.2};
    EXPECT_EQ_ARRAY(a, (enda::array<double, 1> {1.0, 2.0, 3.2}));
}

// ==============================================================

TEST(BasicArrayTest, MoveConstructor)
{
    enda::array<double, 1> A(3);
    A() = 9;

    enda::array<double, 1> B(std::move(A));

    EXPECT_TRUE(A.empty());
    EXPECT_EQ(B.shape(), (enda::shape_t<1> {3}));
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(B(i), 9);
}

// ==============================================================

TEST(BasicArrayTest, MoveAssignment)
{

    enda::array<double, 1> A(3);
    A() = 9;

    enda::array<double, 1> B;
    B = std::move(A);

    EXPECT_TRUE(A.empty());
    EXPECT_EQ(B.shape(), (enda::shape_t<1> {3}));
    for (int i = 0; i < 3; ++i)
        EXPECT_EQ(B(i), 9);
}

// ===================== SWAP =========================================

TEST(BasicArrayTest, StdSwap)
{

    auto V = enda::array<long, 1> {3, 3, 3};
    auto W = enda::array<long, 1> {4, 4, 4, 4};

    std::swap(V, W);

    // V , W are swapped
    EXPECT_EQ(V, (enda::array<long, 1> {4, 4, 4, 4}));
    EXPECT_EQ(W, (enda::array<long, 1> {3, 3, 3}));
}

// ----------------------------------

TEST(BasicArrayTest, SwapView)
{

    auto V = enda::array<long, 1> {3, 3, 3};
    auto W = enda::array<long, 1> {4, 4, 4, 4};

    // swap the view, not the vectors. Views are pointers
    // FIXME should we keep this behaviour ?
    auto VV = V(range(0, 2));
    auto WW = W(range(0, 2));
    swap(VV, WW);

    // V, W unchanged
    EXPECT_EQ(V, (enda::array<long, 1> {3, 3, 3}));
    EXPECT_EQ(W, (enda::array<long, 1> {4, 4, 4, 4}));

    // VV, WW swapped
    EXPECT_EQ(WW, (enda::array<long, 1> {3, 3}));
    EXPECT_EQ(VV, (enda::array<long, 1> {4, 4}));
}

// ----------------------------------

// FIXME Rename as BLAS_SWAP (swap of blas). Only for vector of same size
TEST(BasicArrayTest, DeepSwap)
{
    auto V = enda::array<long, 1> {3, 3, 3};
    auto W = enda::array<long, 1> {4, 4, 4};

    deep_swap(V(), W());

    // V , W are swapped
    EXPECT_EQ(V, (enda::array<long, 1> {4, 4, 4}));
    EXPECT_EQ(W, (enda::array<long, 1> {3, 3, 3}));
}
// ----------------------------------

TEST(BasicArrayTest, DeepSwapView)
{
    auto V = enda::array<long, 1> {3, 3, 3};
    auto W = enda::array<long, 1> {4, 4, 4, 4};

    auto VV = V(range(0, 2));
    auto WW = W(range(0, 2));

    deep_swap(VV, WW);

    // VV, WW swapped
    EXPECT_EQ(WW, (enda::array<long, 1> {3, 3}));
    EXPECT_EQ(VV, (enda::array<long, 1> {4, 4}));

    // V, W changed
    EXPECT_EQ(V, (enda::array<long, 1> {4, 4, 3}));
    EXPECT_EQ(W, (enda::array<long, 1> {3, 3, 4, 4}));
}

// ==============================================================

TEST(BasicArrayTest, Print)
{
    enda::array<long, 2> A(2, 3), B;

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            A(i, j) = 10 * i + j;

    EXPECT_PRINT("\n[[0,1,2]\n [10,11,12]]", A);
}

// ===========   Cross construction  ===================================================

TEST(Array, CrossConstruct1)
{
    enda::array<int, 1> Vi(3);
    Vi() = 3;
    enda::array<double, 1> Vd(Vi);
    EXPECT_ARRAY_NEAR(Vd, Vi);
}

// ------------------
TEST(BasicArrayTest, CrossConstruct2)
{

    enda::array<long, 2> A(2, 3);

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            A(i, j) = 10 * i + j;

    std::vector<enda::array<long, 2>> V(3, A);

    std::vector<enda::array_view<long, 2>> W;
    for (auto& x : V)
        W.emplace_back(x);

    std::vector<enda::array_view<long, 2>> W2(W);

    for (int i = 1; i < 3; ++i)
        V[i] *= i;

    for (int i = 1; i < 3; ++i)
        EXPECT_ARRAY_NEAR(W2[i], i * A);
}

// ------------------

// check non ambiguity of resolution, solved by the check of value type in the constructor
struct A
{};
struct B
{};
std::ostream& operator<<(std::ostream& out, A) { return out; }
std::ostream& operator<<(std::ostream& out, B) { return out; }

int f1(enda::array<A, 1>) { return 1; }
int f1(enda::array<B, 1>) { return 2; }

TEST(BasicArrayTest, CrossConstruct3)
{
    enda::array<A, 1> a(2);
    auto              v = a();
    EXPECT_EQ(f1(v), 1);
}

// =============================================================

TEST(BasicArrayTest, ConvertibleCR)
{

    enda::array<double, 2>   A(2, 2);
    enda::array<dcomplex, 2> B(2, 2);

    // A = B; // should not compile

    B = A;

    auto c = enda::array<dcomplex, 2> {A};

    // can convert an array of double to an array of complex
    static_assert(std::is_constructible_v<enda::array<dcomplex, 2>, enda::array<double, 2>>, "oops");

    // can not do the reverse !
    // static_assert(not std::is_constructible_v<enda::array<double, 2>, enda::array<dcomplex, 2>>, "oops");
}

// =============================================================

TEST(BasicArrayTest, CrossStrideOrder)
{

    // check that = is ok, specially in the contiguous case where we have linear optimisation
    // which should NOT be used in this case ...

    enda::array<long, 3>                 a(2, 3, 4);
    enda::array<long, 3, enda::F_layout> af(2, 3, 4);
    EXPECT_TRUE(af.indexmap().is_contiguous());
    EXPECT_TRUE(af.indexmap().is_strided_1d());

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
            {
                a(i, j, k) = i + 10 * j + 100 * k;
            }

    af = a;

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 4; ++k)
            {
                EXPECT_EQ(af(i, j, k), i + 10 * j + 100 * k);
            }
}

// =============================================================

TEST(BasicArrayTest, Concatenate)
{

    // some dummy arrays
    enda::array<long, 3> a(2, 3, 4);
    enda::array<long, 3> b(2, 3, 5);
    enda::array<long, 3> c(2, 3, 6);

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
        {
            for (int k = 0; k < 4; ++k)
            {
                a(i, j, k) = i + 10 * j + 100 * k;
            }
            for (int k = 0; k < 5; ++k)
            {
                b(i, j, k) = i + 10 * j + 101 * k;
            }
            for (int k = 0; k < 6; ++k)
            {
                c(i, j, k) = i + 10 * j + 102 * k;
            }
        }

    // test concatenate
    auto const abc_axis2_concat = concatenate<2>(a, b, c);
    EXPECT_EQ(abc_axis2_concat.shape()[2], 15);

    for (int i = 0; i < 2; ++i)
        for (int j = 0; j < 3; ++j)
        {
            for (int k = 0; k < 15; ++k)
            {
                if (k < 4)
                    EXPECT_EQ(abc_axis2_concat(i, j, k), a(i, j, k));
                else if (k < 9)
                    EXPECT_EQ(abc_axis2_concat(i, j, k), b(i, j, k - 4));
                else if (k < 15)
                    EXPECT_EQ(abc_axis2_concat(i, j, k), c(i, j, k - 9));
            }
        }
}
