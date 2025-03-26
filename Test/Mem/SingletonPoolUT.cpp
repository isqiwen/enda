#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "Mem/SingletonPool.hpp"

using namespace enda::mem;

TEST(SingletonPoolTest, SingleThreadAllocFree)
{
    ENDA_CREATE_POOL(64B, 6, 4);
    ENDA_INIT_POOL(64B);

    std::vector<char*> blocks;
    for (int i = 0; i < 16; i++)
    {
        char* p = ENDA_GET_POOL(64B).allocate();
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(is_aligned(p, 64), true);
        blocks.push_back(p);
    }

    EXPECT_EQ(ENDA_GET_POOL(64B).allocate(), nullptr);

    for (char* p : blocks)
    {
        ENDA_GET_POOL(64B).deallocate(p);
    }

    char* p = ENDA_GET_POOL(64B).allocate();
    EXPECT_NE(p, nullptr);
    ENDA_GET_POOL(64B).deallocate(p);

    ENDA_GET_POOL(64B).purge_memory();
    blocks.clear();
    for (int i = 0; i < 16; i++)
    {
        char* p = ENDA_GET_POOL(64B).allocate();
        EXPECT_NE(p, nullptr);
        blocks.push_back(p);
    }
    for (char* p : blocks)
    {
        ENDA_GET_POOL(64B).deallocate(p);
    }

    ENDA_GET_POOL(64B).release_memory();
}

TEST(SingletonPoolTest, MultiThreadedAllocFree)
{
    ENDA_CREATE_POOL(128B, 7, 8);
    ENDA_INIT_POOL(128B);

    const int thread_count = 8;
    const int iterations   = 1000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = ENDA_GET_POOL(128B).allocate();
                if (p != nullptr)
                {
                    std::this_thread::yield();
                    ENDA_GET_POOL(128B).deallocate(p);
                    allocCount++;
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thr : threads)
    {
        thr.join();
    }

    EXPECT_GT(allocCount.load(), 0);

    ENDA_GET_POOL(128B).release_memory();
}

TEST(SingletonPoolTest, HighConcurrencyStressTest)
{
    ENDA_CREATE_POOL(256B, 8, 10);
    ENDA_INIT_POOL(256B);

    const int thread_count = 16;
    const int iterations   = 10000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = ENDA_GET_POOL(256B).allocate();
                if (p)
                {
                    ENDA_GET_POOL(256B).deallocate(p);
                    allocCount++;
                }
                else
                {
                    std::this_thread::yield();
                }
            }
        });
    }

    for (auto& thr : threads)
    {
        thr.join();
    }

    EXPECT_GT(allocCount.load(), 0);

    ENDA_GET_POOL(256B).release_memory();
}

TEST(SingletonPoolDeathTest, FreeInvalidPointer)
{
    ENDA_CREATE_POOL(512B, 9, 2);
    ENDA_INIT_POOL(512B);

    char* p = ENDA_GET_POOL(512B).allocate();
    ASSERT_NE(p, nullptr);

    char* misalignedPtr = p + 1;
    EXPECT_DEATH({ ENDA_GET_POOL(512B).deallocate(misalignedPtr); }, "Deallocation error: pointer is not aligned to the start of a block.");

    char* outsidePtr = p - 1;
    EXPECT_DEATH({ ENDA_GET_POOL(512B).deallocate(outsidePtr); }, "Deallocation error: pointer offset out of bounds of the memory pool data region.");

    ENDA_GET_POOL(512B).deallocate(p);

    ENDA_GET_POOL(512B).release_memory();
}
