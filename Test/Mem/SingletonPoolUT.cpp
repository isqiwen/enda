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
    GET_POOL(6).init();
    EXPECT_TRUE(GET_POOL(6).init());

    std::vector<char*> blocks;
    for (int i = 0; i < 16; i++)
    {
        char* p = GET_POOL(6).malloc();
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(is_aligned(p, 64), true);
        blocks.push_back(p);
    }

    EXPECT_EQ(GET_POOL(6).malloc(), nullptr);

    for (char* p : blocks)
    {
        GET_POOL(6).free(p);
    }

    char* p = GET_POOL(6).malloc();
    EXPECT_NE(p, nullptr);
    GET_POOL(6).free(p);

    GET_POOL(6).purge_memory();
    blocks.clear();
    for (int i = 0; i < 16; i++)
    {
        char* p = GET_POOL(6).malloc();
        EXPECT_NE(p, nullptr);
        blocks.push_back(p);
    }
    for (char* p : blocks)
    {
        GET_POOL(6).free(p);
    }
}

/*
TEST(SingletonPoolTest, MultiThreadedAllocFree)
{
    EXPECT_TRUE(TestPool::init());

    const int thread_count = 8;
    const int iterations   = 1000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = TestPool::malloc();
                if (p != nullptr)
                {
                    std::this_thread::yield();
                    TestPool::free(p);
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
}

TEST(SingletonPoolTest, HighConcurrencyStressTest)
{
    EXPECT_TRUE(TestPool::init());

    const int thread_count = 16;
    const int iterations   = 10000;

    std::atomic<int>         allocCount {0};
    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; t++)
    {
        threads.emplace_back([&]() {
            for (int i = 0; i < iterations; i++)
            {
                char* p = TestPool::malloc();
                if (p)
                {
                    for (volatile int j = 0; j < 100; j++)
                    {
                    }
                    TestPool::free(p);
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
}

TEST(SingletonPoolDeathTest, FreeInvalidPointer)
{
    EXPECT_TRUE(TestPool::init());
    char* p = TestPool::malloc();
    ASSERT_NE(p, nullptr);

    char* misalignedPtr = p + 1;
    EXPECT_DEATH({ TestPool::free(misalignedPtr); }, "Deallocation error: pointer is not aligned to the start of a block.");

    char* outsidePtr = TestPool::data_buffer() - 1;
    EXPECT_DEATH({ TestPool::free(outsidePtr); }, "Deallocation error: pointer offset out of bounds of the memory pool data region.");

    TestPool::free(p);
}
*/
