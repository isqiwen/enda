#include "../TestCommon.hpp"

using namespace enda::mem;

// Test fixture class for initializing the test environment
class ConcurrentBitsetTest : public ::testing::Test
{
protected:
    static constexpr uint32_t bit_bound_lg2       = 10; // Bitset size is 2^10 = 1024 bits
    static constexpr uint32_t bit_bound           = 1 << bit_bound_lg2;
    static constexpr uint32_t buffer_size         = concurrent_bitset::buffer_bound_lg2(bit_bound_lg2);
    uint32_t                  buffer[buffer_size] = {0}; // Bitset buffer
    uint32_t                  block_state         = bit_bound_lg2 << concurrent_bitset::state_shift;

    // Initialize the buffer before each test case
    void SetUp() override
    {
        buffer[0] = block_state;

        for (int i = 1; i < buffer_size; ++i)
        {
            buffer[i] = 0;
        }
    }
};

// Single-threaded test: bit acquisition and release
TEST_F(ConcurrentBitsetTest, SingleThreadAcquireRelease)
{
    // Acquire a bit
    std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2);
    ASSERT_EQ(result.first, 0);  // The first acquired bit should be 0
    ASSERT_EQ(result.second, 1); // Used count should be 1

    // Release the bit
    int release_result = concurrent_bitset::release(buffer, 0);
    ASSERT_EQ(release_result, 0); // After release, used count should be 0
}

// Single-threaded test: bit bound
TEST_F(ConcurrentBitsetTest, BitBound)
{
    // Acquire all bits sequentially
    for (uint32_t i = 0; i < bit_bound; ++i)
    {
        std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2);
        ASSERT_EQ(result.first, i);      // Expect acquired bit to be i
        ASSERT_EQ(result.second, i + 1); // Used count should be i + 1
    }
    // Attempt to acquire beyond the bound
    std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2);
    ASSERT_EQ(result.first, -1); // Expect -1 indicating no bits available
    ASSERT_EQ(result.second, -1);
}

// Multi-threaded test: concurrent acquire and release
TEST_F(ConcurrentBitsetTest, MultiThreadAcquireRelease)
{
    const int                num_threads = 16; // Number of threads
    std::vector<std::thread> threads;
    std::vector<int>         acquired_bits(num_threads, -1); // Record acquired bits for each thread

    // Concurrently acquire bits
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&, i]() {
            std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2);
            if (result.first >= 0)
            {
                acquired_bits[i] = result.first; // Record the acquired bit
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }

    // Verify that acquired bits are unique
    std::set<int> unique_bits(acquired_bits.begin(), acquired_bits.end());
    ASSERT_EQ(unique_bits.size(), num_threads); // Ensure no duplicate bits

    // Concurrently release bits
    threads.clear();
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&, i]() {
            if (acquired_bits[i] >= 0)
            {
                concurrent_bitset::release(buffer, acquired_bits[i]);
            }
        });
    }
    for (auto& t : threads)
    {
        t.join();
    }

    // Verify that all bits are correctly released
    for (int i = 0; i < num_threads; ++i)
    {
        std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2, i);
        ASSERT_EQ(result.first, i); // Should reacquire the previously released bit
    }
}

// Performance test: high concurrency acquire and release
TEST_F(ConcurrentBitsetTest, PerformanceTest)
{
    const int                num_threads           = 32;    // Number of threads
    const int                operations_per_thread = 10000; // Operations per thread
    std::vector<std::thread> threads;

    // Record start time
    auto start = std::chrono::high_resolution_clock::now();

    // Start multiple threads to perform acquire and release operations concurrently
    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back([&]() {
            for (int j = 0; j < operations_per_thread; ++j)
            {
                std::pair<int, int> result = concurrent_bitset::acquire_bounded_lg2(buffer, bit_bound_lg2);
                if (result.first >= 0)
                {
                    concurrent_bitset::release(buffer, result.first);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads)
    {
        t.join();
    }

    // Calculate and print the time taken
    auto                          end  = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Time taken for " << num_threads * operations_per_thread << " operations: " << diff.count() << " seconds" << std::endl;
}
