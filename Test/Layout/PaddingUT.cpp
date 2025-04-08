#include "../TestCommon.hpp"

// ----------------- Tests for Extended Euclidean Algorithm -------------------

TEST(ExtendedEuclideanTest, BasicTest)
{
    int x, y;
    int d = ext_euc(30, 12, x, y);
    // The gcd of 30 and 12 is 6.
    EXPECT_EQ(d, 6);
    // Verify that 30 * x + 12 * y equals the gcd.
    EXPECT_EQ(30 * x + 12 * y, d);
}

TEST(ExtendedEuclideanTest, AnotherTest)
{
    int x, y;
    int d = ext_euc(35, 15, x, y);
    // The gcd of 35 and 15 is 5.
    EXPECT_EQ(d, 5);
    EXPECT_EQ(35 * x + 15 * y, d);
}

// ----------------- Tests for Modular Inverse Function -------------------

TEST(ModInverseTest, ValidInverse)
{
    // Test that mod_inverse returns the correct modular inverse when it exists.

    // For mod_inverse(3, 11): 3 * 4 = 12 mod 11 equals 1.
    int inv = mod_inverse(3, 11);
    EXPECT_EQ(inv, 4);

    // For mod_inverse(10, 17): 10 * 12 = 120 mod 17 equals 1.
    inv = mod_inverse(10, 17);
    EXPECT_EQ(inv, 12);

    // For mod_inverse(7, 26): 7 * 15 = 105 mod 26 equals 1.
    inv = mod_inverse(7, 26);
    EXPECT_EQ(inv, 15);

    // For mod_inverse(4, 5): 4 * 4 = 16 mod 5 equals 1.
    inv = mod_inverse(4, 5);
    EXPECT_EQ(inv, 4);
}

TEST(ModInverseTest, NoInverse)
{
    // When a and m are not coprime, no modular inverse exists.
    // For mod_inverse(3, 6), since gcd(3, 6) = 3 ≠ 1, the function should return -1.
    int inv = mod_inverse(3, 6);
    EXPECT_EQ(inv, -1);
}

TEST(GetCacheInfoTest, GetL1DataCacheInfo)
{
    auto cacheInfo = get_L1_data_cache_info();
}

// Test case 1: Trivial case where no padding is needed.
// If the tile size is minimal (D2 = 1 and D1 = 1), there is no conflict vector,
// so the function should return 0.
TEST(ComputePadding2DSetAssocTest, TrivialTileNoPaddingNeeded)
{
    int S       = 8;  // Cache sets
    int A       = 2;  // Associativity
    int D2      = 1;  // Tile height = 1 row
    int D1      = 1;  // Tile width = 1 column
    int M1      = 50; // Unpadded array width
    int B       = 1;  // Cache block size in elements
    int padding = compute_padding_2d_set_assoc(S, A, D2, D1, M1, B);
    EXPECT_EQ(padding, 0);
}

// Test case 2: A general case.
// Using parameters: S = 8, A = 2, D2 = 3, D1 = 5, M1 = 80, B = 1.
// In this test we check that the computed padding is within valid range.
// Since padding is computed as i0 * B and i0 ranges from 0 to S - 1,
// the padding value should be in [0, S*B).
TEST(ComputePadding2DSetAssocTest, GeneralCaseValidPaddingRange)
{
    int S       = 8;
    int A       = 2;
    int D2      = 3;
    int D1      = 5;
    int M1      = 80;
    int B       = 1;
    int padding = compute_padding_2d_set_assoc(S, A, D2, D1, M1, B);
    EXPECT_GE(padding, 0);
    EXPECT_LT(padding, S * B);
}

// Test case 3: Test with a larger block size.
// Suppose the cache block size is 8 elements (for example, a double array with 64-byte cache line).
// Here we use parameters: S = 64, A = 4, D2 = 4, D1 = 32, M1 = 1024, B = 8.
TEST(ComputePadding2DSetAssocTest, LargerBlockSizeTest)
{
    int S       = 64;
    int A       = 4;
    int D2      = 4;
    int D1      = 32;
    int M1      = 1024;
    int B       = 8;
    int padding = compute_padding_2d_set_assoc(S, A, D2, D1, M1, B);
    // The padding result should be a multiple of the block size (B)
    EXPECT_EQ(padding % B, 0);
    // Padding should be non-negative and less than S*B
    EXPECT_GE(padding, 0);
    EXPECT_LT(padding, S * B);
}

// Test case 4: Test scenario where the modular inverse does not exist.
// If a and m are not coprime, mod_inverse returns -1 and that candidate is skipped.
// We use parameters such that i2 and S are not coprime.
TEST(ComputePadding2DSetAssocTest, InverseNonexistentTest)
{
    // Choose S = 10; if i2 is 4 then gcd(4, 10) is 2.
    // With i1 chosen appropriately, the modular inverse may not exist.
    int S  = 10;
    int A  = 2;
    int D2 = 4; // multiple rows
    int D1 = 6;
    int M1 = 100;
    int B  = 1;
    // We are not asserting a specific padding value here,
    // but we ensure the returned result is valid.
    int padding = compute_padding_2d_set_assoc(S, A, D2, D1, M1, B);
    EXPECT_GE(padding, 0);
    EXPECT_LT(padding, S * B);
}

TEST(ComputePadding2DSetAssocTest, LocalMachineTest)
{
    auto cacheInfo = get_L1_data_cache_info();
    int  padding   = compute_padding_2d_set_assoc(cacheInfo.sets, cacheInfo.ways, 3, 4, 256, 16);
    EXPECT_GE(padding, 0);
}
