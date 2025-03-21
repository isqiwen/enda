#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>

#include "Mem/Allocators.hpp"

using namespace enda::mem;

// Fixture to set up common utilities if needed
class AllocatorTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

//--------------------------------------------------------------------------
// Testing mallocator
//--------------------------------------------------------------------------
TEST_F(AllocatorTest, Mallocator_AllocateAndDeallocate)
{
    mallocator<> alloc;
    blk_t        b = alloc.allocate(100);
    ASSERT_NE(b.ptr, nullptr) << "Allocation failed";
    ASSERT_EQ(b.s, 100) << "Size mismatch";
    alloc.deallocate(b);
}

TEST_F(AllocatorTest, Mallocator_ZeroAllocation)
{
    mallocator<> alloc;
    blk_t        b = alloc.allocate(0);
    ASSERT_EQ(b.ptr, nullptr) << "Zero allocation failed";
    alloc.deallocate(b);
}

TEST_F(AllocatorTest, Mallocator_LargeAllocation)
{
    mallocator<> alloc;
    blk_t        b = alloc.allocate(1ULL << 30); // 1GB
    ASSERT_NE(b.ptr, nullptr) << "Large allocation failed";
    alloc.deallocate(b);
}

TEST_F(AllocatorTest, Mallocator_Multithreaded)
{
    mallocator<>             alloc;
    std::vector<std::thread> threads;
    const int                num_threads       = 10;
    const int                allocs_per_thread = 1000;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&alloc]() {
            for (int j = 0; j < allocs_per_thread; ++j)
            {
                blk_t b = alloc.allocate(100);
                ASSERT_NE(b.ptr, nullptr);
                alloc.deallocate(b);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

//--------------------------------------------------------------------------
// Testing MemoryPool
//--------------------------------------------------------------------------
TEST_F(AllocatorTest, MemoryPool_Initialization)
{
    memory_pool pool(1ULL << 20); // 1MB
    ASSERT_GE(pool.capacity(), 1ULL << 20) << "Capacity too small";
    ASSERT_EQ(pool.min_block_size(), 64) << "Default min block size incorrect";
    ASSERT_EQ(pool.max_block_size(), 4096) << "Default max block size incorrect";
}

TEST_F(AllocatorTest, MemoryPool_AllocateAndDeallocate)
{
    memory_pool pool(1ULL << 20);
    blk_t       b = pool.allocate(100);
    ASSERT_NE(b.ptr, nullptr) << "Allocation failed";
    pool.deallocate(b);
}

TEST_F(AllocatorTest, MemoryPool_BoundaryConditions)
{
    memory_pool pool(1ULL << 20);
    blk_t       b1 = pool.allocate(pool.min_block_size());
    blk_t       b2 = pool.allocate(pool.max_block_size());
    ASSERT_NE(b1.ptr, nullptr) << "Min block allocation failed";
    ASSERT_NE(b2.ptr, nullptr) << "Max block allocation failed";
    pool.deallocate(b1);
    pool.deallocate(b2);
}

TEST_F(AllocatorTest, MemoryPool_OversizedAllocation)
{
    memory_pool pool(1ULL << 20);
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    EXPECT_DEATH(pool.allocate(1ULL << 13), "exceeded specified maximum allocation size");
}

TEST_F(AllocatorTest, MemoryPool_Multithreaded)
{
    memory_pool              pool(1ULL << 20);
    std::vector<std::thread> threads;
    const int                num_threads       = 10;
    const int                allocs_per_thread = 1000;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&pool]() {
            for (int j = 0; j < allocs_per_thread; ++j)
            {
                blk_t b = pool.allocate(100);
                ASSERT_NE(b.ptr, nullptr);
                pool.deallocate(b);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

TEST_F(AllocatorTest, MemoryPool_Statistics)
{
    memory_pool                  pool(1ULL << 20);
    blk_t                        b1 = pool.allocate(100);
    blk_t                        b2 = pool.allocate(200);
    memory_pool::usage_statistics stats;
    pool.get_usage_statistics(stats);
    ASSERT_GT(stats.consumed_bytes, 0) << "Consumed bytes should be positive";
    pool.deallocate(b1);
    pool.deallocate(b2);
    pool.get_usage_statistics(stats);
    ASSERT_EQ(stats.consumed_bytes, 0) << "Consumed bytes should be zero after deallocation";
}

//--------------------------------------------------------------------------
// Testing segregator
//--------------------------------------------------------------------------
using SmallAlloc = mallocator<>;
using BigAlloc   = mallocator<>;
using Segregator = segregator<1024, SmallAlloc, BigAlloc>;

TEST_F(AllocatorTest, Segregator_SmallAllocation)
{
    Segregator alloc;
    blk_t      b = alloc.allocate(100); // Below threshold
    ASSERT_NE(b.ptr, nullptr) << "Small allocation failed";
    alloc.deallocate(b);
}

TEST_F(AllocatorTest, Segregator_LargeAllocation)
{
    Segregator alloc;
    blk_t      b = alloc.allocate(2000); // Above threshold
    ASSERT_NE(b.ptr, nullptr) << "Large allocation failed";
    alloc.deallocate(b);
}

TEST_F(AllocatorTest, Segregator_ThresholdBoundary)
{
    Segregator alloc;
    blk_t      b1 = alloc.allocate(1024); // At threshold
    blk_t      b2 = alloc.allocate(1025); // Just above
    ASSERT_NE(b1.ptr, nullptr);
    ASSERT_NE(b2.ptr, nullptr);
    alloc.deallocate(b1);
    alloc.deallocate(b2);
}

TEST_F(AllocatorTest, Segregator_Multithreaded)
{
    Segregator               alloc;
    std::vector<std::thread> threads;
    const int                num_threads       = 10;
    const int                allocs_per_thread = 1000;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&alloc]() {
            for (int j = 0; j < allocs_per_thread; ++j)
            {
                size_t size = (j % 2 == 0) ? 100 : 2000; // Mix small and large
                blk_t  b    = alloc.allocate(size);
                ASSERT_NE(b.ptr, nullptr);
                alloc.deallocate(b);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

//--------------------------------------------------------------------------
// Testing leak_check
//--------------------------------------------------------------------------
using BaseAlloc      = mallocator<>;
using LeakCheckAlloc = leak_check<BaseAlloc>;

TEST_F(AllocatorTest, LeakCheck_NoLeak)
{
    LeakCheckAlloc alloc;
    blk_t          b = alloc.allocate(100);
    alloc.deallocate(b);
    // Destructor should not abort
}

TEST_F(AllocatorTest, LeakCheck_Leak)
{
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
#ifndef NDEBUG
    EXPECT_DEATH(
        {
            LeakCheckAlloc alloc;
            alloc.allocate(100); // Not deallocated
        },
        "Memory leak in allocator");
#endif
}

TEST_F(AllocatorTest, LeakCheck_Multithreaded)
{
    LeakCheckAlloc           alloc;
    std::vector<std::thread> threads;
    const int                num_threads       = 10;
    const int                allocs_per_thread = 1000;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&alloc]() {
            for (int j = 0; j < allocs_per_thread; ++j)
            {
                blk_t b = alloc.allocate(100);
                alloc.deallocate(b);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }
    ASSERT_TRUE(alloc.empty()) << "Memory leak detected";
}

//--------------------------------------------------------------------------
// Testing stats
//--------------------------------------------------------------------------
using BaseAlloc  = mallocator<>;
using StatsAlloc = stats<BaseAlloc>;

TEST_F(AllocatorTest, Stats_Histogram)
{
    StatsAlloc alloc;
    alloc.allocate(0);
    alloc.allocate(1);
    alloc.allocate(2);
    alloc.allocate(3);
    alloc.allocate(4);
    alloc.print_histogram(std::cout);
    std::cout << std::endl;
}

TEST_F(AllocatorTest, Stats_Multithreaded)
{
    StatsAlloc               alloc;
    std::vector<std::thread> threads;
    const int                num_threads       = 10;
    const int                allocs_per_thread = 1000;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&alloc]() {
            for (int j = 0; j < allocs_per_thread; ++j)
            {
                blk_t b = alloc.allocate(100);
                alloc.deallocate(b);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }
}

//--------------------------------------------------------------------------
// Performance Tests
//--------------------------------------------------------------------------
TEST_F(AllocatorTest, Performance_MallocatorVsMemoryPool)
{
    const int    num_allocs = 1000000;
    const size_t size       = 100;

    // Mallocator
    {
        mallocator<> alloc;
        auto         start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_allocs; ++i)
        {
            blk_t b = alloc.allocate(size);
            alloc.deallocate(b);
        }
        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "mallocator: " << diff.count() << " s\n";
    }

    // MemoryPool
    {
        memory_pool pool(1ULL << 30); // 1GB
        auto        start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < num_allocs; ++i)
        {
            blk_t b = pool.allocate(size);
            pool.deallocate(b);
        }
        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "MemoryPool: " << diff.count() << " s\n";
    }
}

TEST_F(AllocatorTest, Performance_Multithreaded)
{
    const int    num_threads       = 10;
    const int    allocs_per_thread = 100000;
    const size_t size              = 100;

    // Mallocator
    {
        mallocator<>             alloc;
        auto                     start = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&alloc, allocs_per_thread, size]() {
                for (int j = 0; j < allocs_per_thread; ++j)
                {
                    blk_t b = alloc.allocate(size);
                    alloc.deallocate(b);
                }
            });
        }
        for (auto& t : threads)
        {
            t.join();
        }
        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "mallocator multithreaded: " << diff.count() << " s\n";
    }

    // MemoryPool
    {
        memory_pool              pool(1ULL << 30); // 1GB
        auto                     start = std::chrono::high_resolution_clock::now();
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i)
        {
            threads.emplace_back([&pool, allocs_per_thread, size]() {
                for (int j = 0; j < allocs_per_thread; ++j)
                {
                    blk_t b = pool.allocate(size);
                    pool.deallocate(b);
                }
            });
        }
        for (auto& t : threads)
        {
            t.join();
        }
        auto                          end  = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "MemoryPool multithreaded: " << diff.count() << " s\n";
    }
}
