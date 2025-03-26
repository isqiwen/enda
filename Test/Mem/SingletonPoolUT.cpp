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
    CREATE_POOL(6, 4);
    EXPECT_TRUE(GET_POOL(6).init());

    std::vector<char*> blocks;
    for (int i = 0; i < 16; i++)
    {
        char* p = GET_POOL(6).allocate();
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(is_aligned(p, 64), true);
        blocks.push_back(p);
    }

    EXPECT_EQ(GET_POOL(6).allocate(), nullptr);

    for (char* p : blocks)
    {
        GET_POOL(6).deallocate(p);
    }

    char* p = GET_POOL(6).allocate();
    EXPECT_NE(p, nullptr);
    GET_POOL(6).deallocate(p);

    GET_POOL(6).purge_memory();
    blocks.clear();
    for (int i = 0; i < 16; i++)
    {
        char* p = GET_POOL(6).allocate();
        EXPECT_NE(p, nullptr);
        blocks.push_back(p);
    }
    for (char* p : blocks)
    {
        GET_POOL(6).deallocate(p);
    }

    GET_POOL(6).release_memory();
}

TEST(SingletonPoolTest, MultiThreadedAllocFree)
{
    CREATE_POOL(7, 10);
    EXPECT_TRUE(GET_POOL(7).init());

    const int thread_count = 8;
    const int iterations   = 1000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = GET_POOL(7).allocate();
                if (p != nullptr)
                {
                    std::this_thread::yield();
                    GET_POOL(7).deallocate(p);
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

    GET_POOL(7).release_memory();
}

TEST(SingletonPoolTest, HighConcurrencyStressTest)
{
    CREATE_POOL(8, 10);
    EXPECT_TRUE(GET_POOL(8).init());

    const int thread_count = 16;
    const int iterations   = 10000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = GET_POOL(8).allocate();
                if (p)
                {
                    GET_POOL(8).deallocate(p);
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

    GET_POOL(8).release_memory();
}

TEST(SingletonPoolDeathTest, FreeInvalidPointer)
{
    CREATE_POOL(9, 2);
    EXPECT_TRUE(GET_POOL(9).init());

    char* p = GET_POOL(9).allocate();
    ASSERT_NE(p, nullptr);

    char* misalignedPtr = p + 1;
    EXPECT_DEATH({ GET_POOL(9).deallocate(misalignedPtr); }, "Deallocation error: pointer is not aligned to the start of a block.");

    char* outsidePtr = p - 1;
    EXPECT_DEATH({ GET_POOL(9).deallocate(outsidePtr); }, "Deallocation error: pointer offset out of bounds of the memory pool data region.");

    GET_POOL(9).deallocate(p);

    GET_POOL(9).release_memory();
}
