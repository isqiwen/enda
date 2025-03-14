#pragma once

#include <atomic>
#include <bit>
#include <cstdint>
#include <limits>
#include <utility>

namespace enda::mem
{
    //==============================================================================
    // Concurrent Bitset
    //
    // This class manages a set of bits and provides atomic operations to acquire,
    // release, and mark bits. Internally, a uint32_t array is used where buffer[0]
    // serves as the header that stores both the usage count (lower bits) and the
    // state header (upper bits).
    //==============================================================================
    struct concurrent_bitset
    {
        static constexpr uint32_t bits_per_int_lg2  = 5;                            // Each uint32_t has 2^5 = 32 bits
        static constexpr uint32_t bits_per_int_mask = (1u << bits_per_int_lg2) - 1; // Mask (0x1F)

        static constexpr uint32_t max_bit_count_lg2 = 25; // Maximum exponent for bit count
        static constexpr uint32_t max_bit_count     = 1u << max_bit_count_lg2;
        static constexpr uint32_t state_shift       = 26; // In header, lower 26 bits hold the used bit count, upper 5 bits hold the state header
        static constexpr uint32_t state_used_mask   = (1u << state_shift) - 1;
        static constexpr uint32_t state_header_mask = uint32_t(0x001f) << state_shift;

        // Compute the number of uint32_t elements required for a bitset with total bits = 1 << bit_bound_lg2 (including header)
        static constexpr uint32_t buffer_bound_lg2(uint32_t bit_bound_lg2) noexcept
        {
            return bit_bound_lg2 <= max_bit_count_lg2 ? 1 + (1u << (bit_bound_lg2 > bits_per_int_lg2 ? bit_bound_lg2 - bits_per_int_lg2 : 0)) : 0;
        }

        // Compute the number of uint32_t elements required for a bitset with total bits = bit_bound (including header)
        static constexpr uint32_t buffer_bound(uint32_t bit_bound) noexcept
        {
            return bit_bound <= max_bit_count ? 1 + (bit_bound >> bits_per_int_lg2) + ((bit_bound & bits_per_int_mask) ? 1 : 0) : 0;
        }

    private:
        // return the index of the first 0-bit in a 32-bit word;
        // if all bits are 1, return -1.
        static inline int bit_first_zero(uint32_t value) noexcept
        {
            if (value == std::numeric_limits<uint32_t>::max())
                return -1;
            return std::countr_zero(~value);
        }

    public:
        // Attempt to acquire a free bit within a range defined by bit_bound_lg2 (i.e., total bits = 1 << bit_bound_lg2)
        // Parameters:
        //   buffer: pointer to an array of uint32_t, where buffer[0] is the header storing state and usage count,
        //           and the subsequent elements store the bit data.
        //   bit_bound_lg2: exponent such that total bits = 1 << bit_bound_lg2
        //   bit: optional starting hint bit index
        //   state_header: optional state header to verify
        // Returns:
        //   On success, returns {acquired bit index, current used bit count + 1}.
        //   On failure, returns (-1,-1), (-2,-2), or (-3,-3) depending on the error.
        static std::pair<int, int> acquire_bounded_lg2(uint32_t* buffer, uint32_t bit_bound_lg2, uint32_t bit = 0, uint32_t state_header = 0) noexcept
        {
            using result_type         = std::pair<int, int>;
            const uint32_t bit_bound  = 1u << bit_bound_lg2;
            const uint32_t word_count = bit_bound >> bits_per_int_lg2;

            if ((max_bit_count_lg2 < bit_bound_lg2) || (state_header & ~state_header_mask) || (bit_bound < bit))
            {
                return {-3, -3};
            }

            std::atomic_ref<uint32_t> state_ref(buffer[0]);
            uint32_t                  state          = state_ref.fetch_add(1, std::memory_order_relaxed);
            bool                      state_error    = (state_header != (state & state_header_mask));
            uint32_t                  state_bit_used = state & state_used_mask;

            if (state_error || (bit_bound <= state_bit_used))
            {
                state_ref.fetch_sub(1, std::memory_order_relaxed);
                return state_error ? result_type {-2, -2} : result_type {-1, -1};
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);

            while (true)
            {
                uint32_t                  word = bit >> bits_per_int_lg2;
                uint32_t                  mask = 1u << (bit & bits_per_int_mask);
                std::atomic_ref<uint32_t> word_ref(buffer[word + 1]);
                uint32_t                  prev = word_ref.fetch_or(mask, std::memory_order_relaxed);

                if ((prev & mask) == 0)
                {
                    return {static_cast<int>(bit), static_cast<int>(state_bit_used + 1)};
                }

                int j = bit_first_zero(prev);
                if (j >= 0)
                {
                    bit = (word << bits_per_int_lg2) | static_cast<uint32_t>(j);
                }
                else
                {
                    bit = (((word + 1) < word_count ? (word + 1) << bits_per_int_lg2 : 0) | (bit & bits_per_int_mask));
                }
            }
        }

        // Release a previously acquired bit.
        // Returns:
        //   On success, returns the current used count (>=0);
        //   returns -1 if the bit was already released;
        //   returns -2 if the state header does not match.
        static std::pair<int, int> acquire_bounded(uint32_t* buffer, uint32_t bit_bound, uint32_t bit = 0, uint32_t state_header = 0) noexcept
        {
            using result_type = std::pair<int, int>;
            if ((max_bit_count < bit_bound) || (state_header & ~state_header_mask) || (bit_bound <= bit))
            {
                return {-3, -3};
            }

            const uint32_t            word_count = bit_bound >> bits_per_int_lg2;
            std::atomic_ref<uint32_t> state_ref(buffer[0]);
            uint32_t                  state          = state_ref.fetch_add(1, std::memory_order_relaxed);
            bool                      state_error    = (state_header != (state & state_header_mask));
            uint32_t                  state_bit_used = state & state_used_mask;

            if (state_error || (bit_bound <= state_bit_used))
            {
                state_ref.fetch_sub(1, std::memory_order_relaxed);
                return state_error ? result_type {-2, -2} : result_type {-1, -1};
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);

            while (true)
            {
                uint32_t                  word = bit >> bits_per_int_lg2;
                uint32_t                  mask = 1u << (bit & bits_per_int_mask);
                std::atomic_ref<uint32_t> word_ref(buffer[word + 1]);
                uint32_t                  prev = word_ref.fetch_or(mask, std::memory_order_relaxed);

                if ((prev & mask) == 0)
                {
                    std::atomic_thread_fence(std::memory_order_seq_cst);
                    return {static_cast<int>(bit), static_cast<int>(state_bit_used + 1)};
                }

                int j = bit_first_zero(prev);
                if (j >= 0)
                {
                    bit = (word << bits_per_int_lg2) | static_cast<uint32_t>(j);
                }
                if ((j < 0) || (bit_bound <= bit))
                {
                    bit = (((word + 1) < word_count ? (word + 1) << bits_per_int_lg2 : 0) | (bit & bits_per_int_mask));
                }
            }
        }

        // Mark a bit as used (similar to release, but uses fetch_or to set the bit).
        // Returns the same error codes as release.
        static int release(uint32_t* buffer, uint32_t bit, uint32_t state_header = 0) noexcept
        {
            std::atomic_ref<uint32_t> state_ref(buffer[0]);
            if (state_header != (state_header_mask & state_ref.load(std::memory_order_relaxed)))
            {
                return -2;
            }

            uint32_t                  mask = 1u << (bit & bits_per_int_mask);
            std::atomic_ref<uint32_t> word_ref(buffer[(bit >> bits_per_int_lg2) + 1]);
            uint32_t                  prev = word_ref.fetch_and(~mask, std::memory_order_relaxed);

            if ((prev & mask) == 0)
            {
                return -1;
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);
            uint32_t count = state_ref.fetch_sub(1, std::memory_order_relaxed);
            std::atomic_thread_fence(std::memory_order_seq_cst);

            return static_cast<int>((count & state_used_mask) - 1);
        }

        static int set(uint32_t* buffer, uint32_t bit, uint32_t state_header = 0) noexcept
        {
            std::atomic_ref<uint32_t> state_ref(buffer[0]);
            if (state_header != (state_header_mask & state_ref.load(std::memory_order_relaxed)))
            {
                return -2;
            }

            uint32_t                  mask = 1u << (bit & bits_per_int_mask);
            std::atomic_ref<uint32_t> word_ref(buffer[(bit >> bits_per_int_lg2) + 1]);
            uint32_t                  prev = word_ref.fetch_or(mask, std::memory_order_relaxed);

            if ((prev & mask) == 0)
            {
                return -1;
            }

            std::atomic_thread_fence(std::memory_order_seq_cst);
            uint32_t count = state_ref.fetch_sub(1, std::memory_order_relaxed);
            return static_cast<int>((count & state_used_mask) - 1);
        }
    };

} // namespace enda::mem
