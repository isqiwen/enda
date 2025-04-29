#include "../TestCommon.hpp"

using namespace enda::mem;

class MemoryPoolTest : public ::testing::Test
{
protected:
    static constexpr uint32_t num_threads = 4;
    static constexpr uint32_t alloc_count = 100000;

    static constexpr size_t lower = 64;                // 64B
    static constexpr size_t upper = 512 * 1024 * 1024; // 512MB

    inline static const double log_lower = std::log(static_cast<double>(lower));
    inline static const double log_upper = std::log(static_cast<double>(upper));

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MemoryPoolTest, CustomMemoryPoolPerformance)
{
    multi_scale_singleton_pool<>::instance().init();

    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([]() {
            std::random_device                     rd;
            std::mt19937                           gen(rd());
            std::uniform_real_distribution<double> dist(MemoryPoolTest::log_lower, MemoryPoolTest::log_upper); // 64B to 512MB

            for (int j = 0; j < MemoryPoolTest::alloc_count; ++j)
            {
                size_t size = std::exp(dist(gen));
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
    ENDA_INIT(stats<multi_scale_singleton_pool<>>::instance());

    std::vector<std::thread> threads;
    auto                     start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([]() {
            std::random_device                     rd;
            std::mt19937                           gen(rd());
            std::uniform_real_distribution<double> dist(MemoryPoolTest::log_lower, MemoryPoolTest::log_upper); // 64B to 512MB

            for (int j = 0; j < MemoryPoolTest::alloc_count; ++j)
            {
                size_t size = std::exp(dist(gen));
                blk_t  blk  = ENDA_MALLOC_STATS(stats<multi_scale_singleton_pool<>>::instance(), size);
                if (blk.ptr)
                {
                    ENDA_FREE(stats<multi_scale_singleton_pool<>>::instance(), blk);
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

    ENDA_RELEASE(stats<multi_scale_singleton_pool<>>::instance());
}
