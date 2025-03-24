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

    void SetUp() override {}

    void TearDown() override {}
};

/*
TEST_F(MemoryPoolTest, BoostMemoryPoolPerformance)
{
    MemoryPool::init();

    constexpr uint32_t       alloc_count = 100000;
    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([alloc_count]() {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_int_distribution<size_t> dist(64, 512 * 1024 * 1024); // 64B to 512MB

            for (int j = 0; j < alloc_count; ++j)
            {
                size_t      size = dist(gen);
                eBlockScale scale;
                void*       ptr = MemoryPool::Malloc(size, scale);
                if (ptr)
                {
                    MemoryPool::Free(ptr, size, scale);
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
*/

TEST_F(MemoryPoolTest, CustomMemoryPoolPerformance)
{
    std::unique_ptr<memory_pool> mp = std::make_unique<memory_pool>(1ULL << 33,           // min_total_alloc_size: 8GB
                                                                    64,                   // min_block_alloc_size: 64 bytes
                                                                    512ULL * 1024 * 1024, // max_block_alloc_size: 512MB
                                                                    512ULL * 1024 * 1024  // min_superblock_size: 512MB
    );

    mp->print_state(std::cout);

    constexpr uint32_t       alloc_count = 100;
    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&mp, alloc_count]() {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_int_distribution<size_t> dist(64, 512 * 1024 * 1024); // 64B to 512MB

            for (int j = 0; j < alloc_count; ++j)
            {
                size_t size = dist(gen);
                blk_t  blk  = mp->allocate(512 * 1024 * 1024);
                if (blk.ptr)
                {
                    mp->deallocate(blk);
                }
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    mp->print_state(std::cout);

    auto end      = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Custom memory_pool: " << duration.count() << " ms\n";
}
