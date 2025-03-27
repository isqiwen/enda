#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "Mem/Allocators.hpp"
#include "Mem/BoostMemoryPool.hpp"

using namespace enda::mem;

class MemoryPoolTest : public ::testing::Test
{
protected:
    static constexpr uint32_t num_threads = 4;
    static constexpr uint32_t alloc_count = 100000;

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MemoryPoolTest, BoostMemoryPoolPerformance)
{
    MemoryPool::init();

    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([]() {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_int_distribution<size_t> dist(64, 512 * 1024 * 1024); // 64B to 512MB

            for (int j = 0; j < MemoryPoolTest::alloc_count; ++j)
            {
                size_t      size = dist(gen);
                eBlockScale scale;
                void*       ptr = MemoryPool::Malloc(size, scale);
                if (ptr)
                {
                    MemoryPool::Free(ptr, size, scale);
                }
                else
                {
                    std::cout << "allocate: " << size << " failed." << std::endl;
                }
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Boost MemoryPool: " << duration.count() << " ms\n";

    MemoryPool::release();
}

TEST_F(MemoryPoolTest, CustomMemoryPoolPerformance)
{
    multi_scale_singleton_pool<>::instance().init();

    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([]() {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_int_distribution<size_t> dist(64, 512 * 1024 * 1024); // 64B to 512MB

            for (int j = 0; j < MemoryPoolTest::alloc_count; ++j)
            {
                size_t size = dist(gen);
                blk_t  blk  = multi_scale_singleton_pool<>::instance().allocate(size);
                if (blk.ptr)
                {
                    multi_scale_singleton_pool<>::instance().deallocate(blk);
                }
                else
                {
                    std::cout << "allocate: " << size << " failed." << std::endl;
                }
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Custom memory_pool: " << duration.count() << " ms\n";

    multi_scale_singleton_pool<>::instance().release();
}

TEST_F(MemoryPoolTest, CustomMemoryPoolStats)
{
    auto pool = stats<multi_scale_singleton_pool<>>();
    pool.init();

    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&pool]() {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_int_distribution<size_t> dist(64, 512 * 1024 * 1024); // 64B to 512MB

            for (int j = 0; j < MemoryPoolTest::alloc_count; ++j)
            {
                size_t size = dist(gen);
                blk_t  blk  = POOL_ALLOC_STATS(pool, size);
                if (blk.ptr)
                {
                    pool.deallocate(blk);
                }
                else
                {
                    std::cout << "allocate: " << size << " failed." << std::endl;
                }
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Custom memory_pool: " << duration.count() << " ms\n";

    pool.release();
}
