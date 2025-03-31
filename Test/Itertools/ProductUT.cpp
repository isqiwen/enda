#include <Itertools/Product.hpp>
#include <Itertools/RangeView.hpp>
#include <algorithm>
#include <gtest/gtest.h>
#include <iterator>
#include <tuple>
#include <vector>

//------------------------------------------------------------------------------
// Test the product_iterator construction manually.
// This test constructs a three‑level product iterator over arrays X, Y, Z.
// Expected output for X = {1,2,3}, Y = {'A','B'}, Z = {0.5, 0.6} is:
//   (1, 'A', 0.5), (1, 'A', 0.6),
//   (1, 'B', 0.5), (1, 'B', 0.6),
//   (2, 'A', 0.5), (2, 'A', 0.6),
//   (2, 'B', 0.5), (2, 'B', 0.6),
//   (3, 'A', 0.5), (3, 'A', 0.6),
//   (3, 'B', 0.5), (3, 'B', 0.6).
//------------------------------------------------------------------------------
TEST(ProductIteratorTest, ManualConstruction)
{
    int    X[3] = {1, 2, 3};
    char   Y[2] = {'A', 'B'};
    double Z[2] = {0.5, 0.6};

    // Construct the innermost product iterator for Z.
    auto it_z_begin = enda::itertools::product_iterator<double*>(std::begin(Z), std::end(Z));
    auto it_z_end   = enda::itertools::product_iterator<double*>(std::end(Z), std::end(Z));

    // Construct the middle level iterator for Y and Z.
    auto it_yz_begin = enda::itertools::product_iterator<char*, double*>(std::begin(Y), std::end(Y), it_z_begin, it_z_begin, it_z_end);
    auto it_yz_end   = enda::itertools::product_iterator<char*, double*>(std::end(Y), std::end(Y), it_z_end, it_z_begin, it_z_end);

    // Construct the outer level iterator for X, Y, and Z.
    auto it_xyz_begin = enda::itertools::product_iterator<int*, char*, double*>(std::begin(X), std::end(X), it_yz_begin, it_yz_begin, it_yz_end);
    auto it_xyz_end   = enda::itertools::product_iterator<int*, char*, double*>(std::end(X), std::end(X), it_yz_end, it_yz_begin, it_yz_end);

    std::vector<std::tuple<int, char, double>> result;
    for (auto tup : enda::itertools::range_view(it_xyz_begin, it_xyz_end))
    {
        result.push_back(tup);
    }

    std::vector<std::tuple<int, char, double>> expected = {{1, 'A', 0.5},
                                                           {1, 'A', 0.6},
                                                           {1, 'B', 0.5},
                                                           {1, 'B', 0.6},
                                                           {2, 'A', 0.5},
                                                           {2, 'A', 0.6},
                                                           {2, 'B', 0.5},
                                                           {2, 'B', 0.6},
                                                           {3, 'A', 0.5},
                                                           {3, 'A', 0.6},
                                                           {3, 'B', 0.5},
                                                           {3, 'B', 0.6}};

    EXPECT_EQ(result, expected);
}

//------------------------------------------------------------------------------
// Test the high-level product function.
// This test uses vectors X, Y, Z and the enda::itertools::product function to
// generate the Cartesian product. For X = {1,2,3}, Y = {'A','B'}, Z = {0.5, 100.0},
// the expected output is:
//   (1, 'A', 0.5), (1, 'A', 100.0),
//   (1, 'B', 0.5), (1, 'B', 100.0),
//   (2, 'A', 0.5), (2, 'A', 100.0),
//   (2, 'B', 0.5), (2, 'B', 100.0),
//   (3, 'A', 0.5), (3, 'A', 100.0),
//   (3, 'B', 0.5), (3, 'B', 100.0).
//------------------------------------------------------------------------------
TEST(ProductTest, HighLevelProduct)
{
    std::vector<int>    X = {1, 2, 3};
    std::vector<char>   Y = {'A', 'B'};
    std::vector<double> Z = {0.5, 100.0};

    std::vector<std::tuple<int, char, double>> result;
    for (auto tup : enda::itertools::product(X, Y, Z))
    {
        result.push_back(tup);
    }

    std::vector<std::tuple<int, char, double>> expected = {{1, 'A', 0.5},
                                                           {1, 'A', 100.0},
                                                           {1, 'B', 0.5},
                                                           {1, 'B', 100.0},
                                                           {2, 'A', 0.5},
                                                           {2, 'A', 100.0},
                                                           {2, 'B', 0.5},
                                                           {2, 'B', 100.0},
                                                           {3, 'A', 0.5},
                                                           {3, 'A', 100.0},
                                                           {3, 'B', 0.5},
                                                           {3, 'B', 100.0}};

    EXPECT_EQ(result, expected);
}
