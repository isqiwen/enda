#include "TestCommon.hpp"

TEST(Array, InitializerOfArray)
{ // NOLINT
    auto pts = enda::array<std::array<double, 2>, 1> {{0.0, 0.0}, {0.0, 1.0}, {1.0, 0.0}, {1.0, 1.0}};
    EXPECT_EQ(pts.shape(), (enda::shape_t<1> {4}));
}

TEST(Matrix, Create1)
{ // NOLINT
    enda::matrix<long> a(3, 3);
    EXPECT_EQ(a.shape(), (enda::shape_t<2> {3, 3}));

    enda::matrix<long> b {3, 3}; // fine, no ambiguity with init list
    EXPECT_EQ(b.shape(), (enda::shape_t<2> {3, 3}));

    enda::matrix<long> c {{3, 3}}; // Careful ?
    EXPECT_EQ(c.shape(), (enda::shape_t<2> {1, 2}));
}

// ===============================================================

TEST(Matrix, Create2)
{ // NOLINT
    auto m1 = matrix<double> {{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}};
    EXPECT_EQ(m1.shape(), (enda::shape_t<2> {5, 2}));

    auto m2 = matrix<double> {{{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}}};
    EXPECT_EQ(m2.shape(), (enda::shape_t<2> {5, 2}));
}

TEST(Matrix, Create2Complex)
{ // NOLINT
    auto m3 = matrix<dcomplex> {{-10, -3i}, {12, 14}, {14, 12}, {16, 16}, {18, 16}};
    EXPECT_EQ(m3.shape(), (enda::shape_t<2> {5, 2}));

    auto m4 = matrix<dcomplex> {{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}};
    EXPECT_EQ(m4.shape(), (enda::shape_t<2> {5, 2}));

    auto m5 = matrix<dcomplex> {{{-10, -3i}, {12, 14}, {14, 12}, {16, 16}, {18, 16}}};
    EXPECT_EQ(m5.shape(), (enda::shape_t<2> {5, 2}));

    auto m6 = matrix<dcomplex> {{{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}}};
    EXPECT_EQ(m6.shape(), (enda::shape_t<2> {1, 5})); /// VERY DANGEROUS !
}

// ===============================================================

TEST(Matrix, MoveToArray)
{ // NOLINT

    auto m = matrix<double> {{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}};

    auto mcopy = m;

    auto a = enda::array<double, 2> {std::move(m)};

    EXPECT_EQ_ARRAY(mcopy, matrix_view<double> {a}); // should be EXACLY equal

    // HEAP only, it would not be correct for the other cases
#if !defined(NDA_TEST_DEFAULT_ALLOC_SSO) and !defined(NDA_TEST_DEFAULT_ALLOC_MBUCKET)
    EXPECT_EQ(m.storage().size(), 0);
    EXPECT_TRUE(m.storage().is_null());
#endif
}

// ===============================================================

TEST(Matrix, MoveFromArray)
{ // NOLINT

    auto a = enda::array<double, 2> {{-10, -3}, {12, 14}, {14, 12}, {16, 16}, {18, 16}};

    auto acopy = a;

    auto m = matrix<double> {std::move(a)};

    EXPECT_EQ_ARRAY(m, matrix_view<double> {acopy}); // should be EXACLY equal

    // HEAP only, it would not be correct for the other cases
#if !defined(NDA_TEST_DEFAULT_ALLOC_SSO) and !defined(NDA_TEST_DEFAULT_ALLOC_MBUCKET)
    EXPECT_EQ(a.storage().size(), 0);
    EXPECT_TRUE(a.storage().is_null());
#endif
}

// ===============================================================

TEST(Matrix, Transpose)
{ // NOLINT

    const int N = 5;

    enda::matrix<double, F_layout>     A(N, N);
    enda::matrix<std::complex<double>> B(N, N);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            A(i, j) = i + 2 * j + 1;
            B(i, j) = i + 2.5 * j + (i - 0.8 * j) * 1i;
        }

    auto at = transpose(A);
    auto bt = transpose(B);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            EXPECT_COMPLEX_NEAR(at(i, j), A(j, i));
            EXPECT_COMPLEX_NEAR(bt(i, j), B(j, i));
        }
}
// ===============================================================

TEST(Matrix, Dagger)
{ // NOLINT

    const int N = 5;

    enda::matrix<double, F_layout>     A(N, N);
    enda::matrix<std::complex<double>> B(N, N);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            A(i, j) = i + 2 * j + 1;
            B(i, j) = i + 2.5 * j + (i - 0.8 * j) * 1i;
        }

    auto ad = dagger(A);
    auto bd = dagger(B);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            EXPECT_COMPLEX_NEAR(ad(i, j), A(j, i));
            EXPECT_COMPLEX_NEAR(bd(i, j), std::conj(B(j, i)));
        }
}

// ===============================================================

TEST(Matrix, DaggerSlice)
{ // NOLINT

    const int N = 5, Nb = 2;

    enda::matrix<std::complex<double>> a(N, N);
    enda::matrix<std::complex<double>> b(Nb, Nb);

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            a(i, j) = i + 2.5 * j + (i - 0.8 * j) * 1i;
        }
    for (int i = 0; i < Nb; ++i)
        for (int j = 0; j < Nb; ++j)
        {
            b(j, i) = std::conj(a(i, j));
        }

    auto a1 = conj(enda::matrix_view<dcomplex> {a});
    EXPECT_EQ(a1.shape(), (enda::shape_t<2> {N, N}));

    auto ad = dagger(a)(range(0, 2), range(0, 2));
    EXPECT_EQ(ad.shape(), (enda::shape_t<2> {Nb, Nb}));

    EXPECT_ARRAY_NEAR(ad, b, 1.e-14);
}

// ===============================================================

TEST(Matrix, Eye)
{ // NOLINT

    EXPECT_EQ_ARRAY(enda::eye<long>(3), (enda::matrix<long> {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}));
}

// ===============================================================

TEST(Matrix, Diagonal)
{ // NOLINT
    auto v = enda::vector<int> {1, 2, 3};
    auto m = enda::diag(v);
    EXPECT_EQ_ARRAY(m, (enda::matrix<int> {{1, 0, 0}, {0, 2, 0}, {0, 0, 3}}));
    EXPECT_EQ_ARRAY(enda::diagonal(m), v);

    enda::diagonal(m) += v;
    EXPECT_EQ_ARRAY(enda::diagonal(m), 2 * v);
}

// ===============================================================

TEST(Matrix, Slice)
{ // NOLINT

    const int N = 10;

    enda::matrix<double> a(N, N);

    enda::range R(2, 4);

    auto v = a(R, 7);

    static_assert(decltype(v)::layout_t::layout_prop == enda::layout_prop_e::strided_1d, "ee");
    static_assert(enda::has_contiguous(decltype(v)::layout_t::layout_prop) == false, "ee");
}

TEST(Matrix, Algebra)
{
    auto m1 = enda::matrix<double> {{1, 2}, {3, 4}};
    auto m2 = enda::matrix<double> {{1, 2}, {2, 1}};

    auto prod = enda::matrix<double> {{5, 4}, {11, 10}};
    EXPECT_EQ(prod, make_regular(m1 * m2));

    EXPECT_EQ(make_regular(m1 / m2), make_regular(m1 * inverse(m2)));
}
