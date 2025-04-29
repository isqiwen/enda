#include "../TestCommon.hpp"

using namespace enda::mem;

//------------------------------------------------------------------------------
// Test 1: Basic memory allocation test using mallocator
//------------------------------------------------------------------------------
TEST(MallocatorTest, BasicAllocation)
{
    // Allocate 1024 bytes.
    blk_t blk = mallocator<>::allocate(1024);
    ASSERT_NE(blk.ptr, nullptr) << "mallocator allocation failed";
    ASSERT_EQ(blk.requested_size, 1024u);

    // Write a fixed pattern to the memory and then verify.
    memset<Host>(blk.ptr, 0xAA, 1024);
    for (std::size_t i = 0; i < 1024; ++i)
    {
        ASSERT_EQ(static_cast<unsigned char>(blk.ptr[i]), 0xAA);
    }
    mallocator<>::deallocate(blk);
}

//------------------------------------------------------------------------------
// Test 2: Test allocate_zero and verify that the memory is zero-initialized.
//------------------------------------------------------------------------------
TEST(MallocatorTest, ZeroAllocation)
{
    blk_t blk = mallocator<>::allocate_zero(1024);
    ASSERT_NE(blk.ptr, nullptr) << "allocate_zero returned a null pointer";
    for (std::size_t i = 0; i < 1024; ++i)
    {
        ASSERT_EQ(static_cast<unsigned char>(blk.ptr[i]), 0);
    }
    mallocator<>::deallocate(blk);
}

//------------------------------------------------------------------------------
// Test 3: Multi-scale pool boundary condition tests.
//------------------------------------------------------------------------------
TEST(MultiScalePoolTest, BoundaryAllocations)
{
    // Initialize all memory pools.
    multi_scale_singleton_pool<>::init();

    // Test allocations for boundary conditions:
    // 0, exactly equal to each threshold, and a size slightly larger than the maximum predefined block (512M).
    std::vector<std::size_t> sizes = {
        0,
        _64K,
        _1M,
        _2M,
        _4M,
        _8M,
        _16M,
        _32M,
        _64M,
        _128M,
        _256M,
        _512M,
        _512M + 1 // This size should use eDirect allocation.
    };

    for (auto size : sizes)
    {
        blk_t blk = multi_scale_singleton_pool<>::allocate(size);

        // If the allocation size exceeds 512M, the returned scale should be eDirect.
        if (size > _512M)
        {
            EXPECT_EQ(blk.scale, BlockScale::eDirect) << "Large allocation should use eDirect allocation";
        }
        else
        {
            EXPECT_NE(blk.scale, BlockScale::eDirect) << "Predefined block allocation should not use eDirect";
        }

        // For non-zero allocations, verify that memory is writable.
        if (size > 0)
        {
            ASSERT_NE(blk.ptr, nullptr) << "Allocation of " << size << " bytes failed";
            memset<Host>(blk.ptr, 0xBB, size);
            for (std::size_t i = 0; i < size; ++i)
            {
                ASSERT_EQ(static_cast<unsigned char>(blk.ptr[i]), 0xBB);
            }
        }
        multi_scale_singleton_pool<>::deallocate(blk);
    }
    multi_scale_singleton_pool<>::release();
}

//------------------------------------------------------------------------------
// Test 4: Test the statistics wrapper to track allocation statistics.
//------------------------------------------------------------------------------
TEST(StatsTest, AllocationStatistics)
{
    // Create a stats wrapper (which internally uses mallocator's static interface).
    stats<mallocator<>>& statAlloc = stats<mallocator<>>::instance();
    statAlloc.init();

    {
        blk_t blk1 = statAlloc.allocate(1024, __FILE__, __LINE__);
        blk_t blk2 = statAlloc.allocate_zero(2048, __FILE__, __LINE__);
        ASSERT_NE(blk1.ptr, nullptr);
        ASSERT_NE(blk2.ptr, nullptr);

        // Write data to blk1 and verify correctness.
        memset<Host>(blk1.ptr, 0xCC, 1024);
        for (std::size_t i = 0; i < 1024; ++i)
        {
            ASSERT_EQ(static_cast<unsigned char>(blk1.ptr[i]), 0xCC);
        }

        // Verify that the total allocated memory is greater than 0.
        EXPECT_GT(statAlloc.get_memory_used(), 0);
        statAlloc.deallocate(blk1);
        statAlloc.deallocate(blk2);
    }
    EXPECT_TRUE(statAlloc.empty()) << "There should be no memory leak; all allocations must be freed";
    statAlloc.release();
}

//------------------------------------------------------------------------------
// Test 5: Multi-threaded concurrent allocation and deallocation test.
//------------------------------------------------------------------------------
TEST(MultiThreadingTest, ConcurrentAllocation)
{
    multi_scale_singleton_pool<>::init();
    const int                threadCount          = 8;
    const int                allocationsPerThread = 100;
    std::vector<std::thread> threads;
    std::mutex               vecMutex;
    std::vector<blk_t>       allocatedBlocks;

    auto allocateTask = [&]() {
        std::vector<blk_t> localBlocks;
        for (int i = 0; i < allocationsPerThread; ++i)
        {
            // Randomly allocate between 1024 and 2047 bytes.
            std::size_t size = 1024 + (std::rand() % 1024);
            blk_t       blk  = multi_scale_singleton_pool<>::allocate(size);
            if (blk.ptr)
            {
                // Fill the memory with a pattern for later verification.
                memset<Host>(blk.ptr, 0xDD, size);
            }
            localBlocks.push_back(blk);
        }
        {
            std::scoped_lock lock(vecMutex);
            allocatedBlocks.insert(allocatedBlocks.end(), localBlocks.begin(), localBlocks.end());
        }
    };

    // Launch multiple threads.
    for (int t = 0; t < threadCount; ++t)
    {
        threads.emplace_back(allocateTask);
    }
    for (auto& th : threads)
    {
        th.join();
    }

    // Verify the pattern in each allocated block and then free it.
    for (auto& blk : allocatedBlocks)
    {
        if (blk.ptr)
        {
            for (std::size_t i = 0; i < blk.requested_size; ++i)
            {
                ASSERT_EQ(static_cast<unsigned char>(blk.ptr[i]), 0xDD);
            }
            multi_scale_singleton_pool<>::deallocate(blk);
        }
    }
    multi_scale_singleton_pool<>::release();
}

//------------------------------------------------------------------------------
// Test 6: Test macros for allocation and deallocation.
//------------------------------------------------------------------------------
TEST(MacroTest, ENDA_MALLOC_AND_FREE)
{
    // Since the allocators are singletons, get the instance from mallocator.
    auto& alloc = mallocator<>::instance();
    ENDA_INIT(alloc);
    blk_t blk = ENDA_MALLOC(alloc, 512);
    ASSERT_NE(blk.ptr, nullptr);
    // Write data to the allocated block and verify before deallocation.
    memset<Host>(blk.ptr, 0xEE, 512);
    for (std::size_t i = 0; i < 512; ++i)
    {
        ASSERT_EQ(static_cast<unsigned char>(blk.ptr[i]), 0xEE);
    }
    ENDA_FREE(alloc, blk);
    ENDA_RELEASE(alloc);
}
