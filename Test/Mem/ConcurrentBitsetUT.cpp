#include <gtest/gtest.h>
#include <vector>

#include "Mem/ConcurrentBitset.hpp"

using namespace enda::mem;

TEST(ConcurrentBitsetTest, BasicAcquireReleaseLg2)
{
    constexpr uint32_t    bit_bound_lg2 = 6;
    const uint32_t        buffer_size   = concurrent_bitset::buffer_bound_lg2(bit_bound_lg2);
    std::vector<uint32_t> buffer(buffer_size, 0);

    auto res = concurrent_bitset::acquire_bounded_lg2(buffer.data(), bit_bound_lg2, 0, 0);
    EXPECT_GE(res.first, 0);
    EXPECT_EQ(res.second, 1);

    int release_result = concurrent_bitset::release(buffer.data(), res.first, 0);
    EXPECT_EQ(release_result, 0);

    release_result = concurrent_bitset::release(buffer.data(), res.first, 0);
    EXPECT_EQ(release_result, -1);
}

TEST(ConcurrentBitsetTest, FullBitsetAcquire)
{
    constexpr uint32_t    bit_bound_lg2 = 5;
    const uint32_t        buffer_size   = concurrent_bitset::buffer_bound_lg2(bit_bound_lg2);
    std::vector<uint32_t> buffer(buffer_size, 0);

    const uint32_t total_bits = 1u << bit_bound_lg2;
    for (uint32_t i = 0; i < total_bits; ++i)
    {
        auto res = concurrent_bitset::acquire_bounded_lg2(buffer.data(), bit_bound_lg2, 0, 0);
        EXPECT_GE(res.first, 0);
    }

    auto res_fail = concurrent_bitset::acquire_bounded_lg2(buffer.data(), bit_bound_lg2, 0, 0);
    EXPECT_EQ(res_fail.first, -1);
    EXPECT_EQ(res_fail.second, -1);
}

TEST(ConcurrentBitsetTest, StateHeaderMismatch)
{
    constexpr uint32_t    bit_bound_lg2 = 6;
    const uint32_t        buffer_size   = concurrent_bitset::buffer_bound_lg2(bit_bound_lg2);
    std::vector<uint32_t> buffer(buffer_size, 0);

    auto res = concurrent_bitset::acquire_bounded_lg2(buffer.data(), bit_bound_lg2, 0, 0);
    EXPECT_GE(res.first, 0);

    int release_result = concurrent_bitset::release(buffer.data(), res.first, 1);
    EXPECT_EQ(release_result, -2);
}

TEST(ConcurrentBitsetTest, TestSetFunction)
{
    constexpr uint32_t    bit_bound_lg2 = 6;
    const uint32_t        buffer_size   = concurrent_bitset::buffer_bound_lg2(bit_bound_lg2);
    std::vector<uint32_t> buffer(buffer_size, 0);

    auto res = concurrent_bitset::acquire_bounded_lg2(buffer.data(), bit_bound_lg2, 0, 0);
    EXPECT_GE(res.first, 0);
    int bit_index = res.first;

    int set_result = concurrent_bitset::set(buffer.data(), bit_index, 0);
    EXPECT_EQ(set_result, 0);

    int set_fail = concurrent_bitset::set(buffer.data(), 1, 0);
    EXPECT_EQ(set_fail, -1);
}

TEST(ConcurrentBitsetTest, AcquireBoundedTest)
{
    constexpr uint32_t    bit_bound   = 100;
    const uint32_t        buffer_size = concurrent_bitset::buffer_bound(bit_bound);
    std::vector<uint32_t> buffer(buffer_size, 0);

    auto res = concurrent_bitset::acquire_bounded(buffer.data(), bit_bound, 0, 0);
    EXPECT_GE(res.first, 0);
    EXPECT_EQ(res.second, 1);
}
