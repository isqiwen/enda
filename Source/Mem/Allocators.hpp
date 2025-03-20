#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#include "Exceptions.hpp"
#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/ConcurrentBitset.hpp"
#include "Mem/Malloc.hpp"
#include "Mem/Memset.hpp"

#ifndef NDEBUG
    #include <iostream>
#endif

namespace enda::mem
{
    // Memory block consisting of a pointer and its size.
    struct blk_t
    {
        // Pointer to the memory block.
        char* ptr = nullptr;

        // Size of the memory block in bytes.
        size_t s = 0;
    };

    /**
     * @brief Custom allocator that uses enda::mem::malloc to allocate memory.
     * @tparam AdrSp enda::mem::AddressSpace in which the memory is allocated.
     */
    template<AddressSpace AdrSp = Host>
    class mallocator : public immovable<mallocator<AdrSp>>
    {
    public:
        // enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = AdrSp;

        // alloc_size: Size in bytes of the memory to allocate.
        static blk_t allocate(size_t alloc_size) noexcept { return blk_t {(char*)malloc<address_space>(alloc_size), alloc_size}; }

        static blk_t allocate_zero(size_t alloc_size) noexcept
        {
            blk_t b = allocate(alloc_size);
            memset<address_space>((void*)b.ptr, 0, alloc_size);
            return b;
        }

        static void deallocate(blk_t b) noexcept { free<address_space>((void*)b.ptr); }
    };

    // Compute the smallest exponent 'exp' such that 2^exp >= n.
    constexpr uint32_t integral_power_of_two_that_contains(size_t n) noexcept
    {
        uint32_t exp   = 0;
        size_t   power = 1;
        while (power < n)
        {
            ++exp;
            power <<= 1;
        }
        return exp;
    }

    constexpr uint64_t clock_tic() noexcept
    {
#if defined(__i386__) || defined(__x86_64)

        // Return value of 64-bit hi-res clock register.

        unsigned a = 0, d = 0;

        __asm__ volatile("rdtsc" : "=a"(a), "=d"(d));

        return ((uint64_t)a) | (((uint64_t)d) << 32);
#else
        return std::chrono::high_resolution_clock::now().time_since_epoch().count();
#endif
    }

    //==============================================================================
    // MemoryPool
    //
    // The MemoryPool divides a large memory block into several "superblocks", each
    // of which is subdivided into equally sized "blocks". Each superblock maintains
    // a concurrent bitset state to mark whether each block is allocated.
    //
    // +--------------------------------------------------------------+
    // |                        Memory Pool                           |
    // |                                                              |
    // |  +-----------------------+----------------------------------+  ← Header Region (Status Region + Hint Array)
    // |  |  Superblock State     |            Hint Array            |
    // |  |      Area             |  (One entry per block size,      |
    // |  | (for each superblock) |  repeated s_hint_per_block_size) |
    // |  +-----------------------+----------------------------------+  ← m_data_offset (header size)
    // |                                                              |
    // +--------------------------------------------------------------+
    // |                        Data Region                           |
    // |                                                              |
    // |  +------------------+   +------------------+   +---------+  ← Superblocks
    // |  |  Superblock 0    |   |  Superblock 1    |   |  ...    |
    // |  |   (1 << m_sb_size_lg2 bytes)   |   (1 << m_sb_size_lg2 bytes)      |
    // |  |  +------------+  |   |  +------------+  |              |
    // |  |  |  Block 0   |  |   |  |  Block 0   |  |              |
    // |  |  |  Block 1   |  |   |  |  Block 1   |  |              |
    // |  |  |   ...      |  |   |  |   ...      |  |              |
    // |  |  +------------+  |   |  +------------+  |              |
    // |  +------------------+   +------------------+              |
    // |                                                              |
    // +--------------------------------------------------------------+
    //
    //==============================================================================
    class MemoryPool : public immovable<MemoryPool>
    {
    public:
        // Structure to hold memory pool usage statistics.
        struct usage_statistics
        {
            size_t capacity_bytes;       // Total capacity in bytes
            size_t superblock_bytes;     // Bytes per superblock
            size_t max_block_bytes;      // Maximum block size in bytes
            size_t min_block_bytes;      // Minimum block size in bytes
            size_t capacity_superblocks; // Number of superblocks
            size_t consumed_superblocks; // Superblocks assigned to allocations
            size_t consumed_blocks;      // Number of allocated blocks
            size_t consumed_bytes;       // Total allocated bytes
            size_t reserved_blocks;      // Unallocated blocks in superblocks
            size_t reserved_bytes;       // Unallocated bytes in superblocks
        };

        // Only `Host` enda::mem::AddressSpace is supported for this allocator.
        static constexpr auto address_space = Host;

        //--------------------------------------------------------------------------
        // Constructor:
        //   min_total_alloc_size: Minimum total bytes that the memory pool must provide.
        //   min_block_alloc_size: Minimum block allocation size in bytes (default 64 bytes).
        //   max_block_alloc_size: Maximum block allocation size in bytes (default 4KB).
        //   min_superblock_size: Minimum superblock size in bytes (default 1MB).
        //--------------------------------------------------------------------------
        MemoryPool(size_t min_total_alloc_size, size_t min_block_alloc_size = 0, size_t max_block_alloc_size = 0, size_t min_superblock_size = 0)
        {
            // Alignment requirements: align to 16 uint32_t elements.
            constexpr uint32_t int_align_lg2               = 4;
            constexpr uint32_t int_align_mask              = (1u << int_align_lg2) - 1;
            constexpr uint32_t default_min_block_size      = 1u << 6;  // 64 bytes
            constexpr uint32_t default_max_block_size      = 1u << 12; // 4096 bytes
            constexpr uint32_t default_min_superblock_size = 1u << 20; // 1MB bytes

            //--------------------------------------------------
            // Default block and superblock sizes:

            if (0 == min_block_alloc_size)
            {
                min_superblock_size  = std::min(default_min_superblock_size, static_cast<uint32_t>(min_total_alloc_size));
                min_block_alloc_size = std::min(default_min_block_size, static_cast<uint32_t>(min_superblock_size));
                max_block_alloc_size = std::min(default_max_block_size, static_cast<uint32_t>(min_superblock_size));
            }
            else if (0 == min_superblock_size)
            {
                // Choose superblock size as minimum of:
                //   max_block_per_superblock * min_block_size
                //   max_superblock_size
                //   min_total_alloc_size
                size_t max_superblock_size = min_block_alloc_size * concurrent_bitset::max_bit_count;
                min_superblock_size        = std::min(max_superblock_size, std::min((size_t)s_max_superblock_size, min_total_alloc_size));
            }

            if (0 == max_block_alloc_size)
            {
                max_block_alloc_size = min_superblock_size;
            }

            memory_pool_bounds_verification(
                min_block_alloc_size, max_block_alloc_size, min_superblock_size, s_max_superblock_size, concurrent_bitset::max_bit_count, min_total_alloc_size);

            //--------------------------------------------------
            // Block and superblock size is power of two:
            m_min_block_size_lg2 = integral_power_of_two_that_contains(min_block_alloc_size);
            m_max_block_size_lg2 = integral_power_of_two_that_contains(max_block_alloc_size);
            m_sb_size_lg2        = integral_power_of_two_that_contains(min_superblock_size);

            // Compute the number of superblocks needed to cover min_total_alloc_size.
            uint64_t sb_size = 1UL << m_sb_size_lg2;
            m_sb_count       = static_cast<int32_t>((min_total_alloc_size + sb_size - 1) >> m_sb_size_lg2);

            // Compute the size of each superblock's state area using concurrent_bitset::buffer_bound_lg2.
            uint32_t max_block_count_lg2 = m_sb_size_lg2 - m_min_block_size_lg2;
            uint32_t cb_size             = concurrent_bitset::buffer_bound_lg2(max_block_count_lg2);
            // Round a value X up to the smallest multiple of the alignment boundary A (where A must be a power of 2):
            // aligned(X,A) = (X+(A−1)) & ∼(A−1)
            m_sb_state_size = (cb_size + int_align_mask) & ~int_align_mask;

            // Compute the total state area size (in uint32_t elements).
            uint32_t all_sb_state_size = (m_sb_count * m_sb_state_size + int_align_mask) & ~int_align_mask;

            // Compute the number of different block sizes.
            int32_t number_block_sizes = 1 + m_max_block_size_lg2 - m_min_block_size_lg2;
            // Hint array size: one uint32_t per block size, aligned.
            int32_t block_size_array_size = (number_block_sizes + int_align_mask) & ~int_align_mask;

            m_hint_offset = all_sb_state_size;
            m_data_offset = m_hint_offset + block_size_array_size * s_hint_per_block_size;

            // Compute total allocation size: header area + superblock data area.
            size_t header_size      = m_data_offset * sizeof(uint32_t);
            size_t alloc_size_total = header_size + (static_cast<size_t>(m_sb_count) << m_sb_size_lg2);

            m_buffer = (char*)malloc<address_space>(alloc_size_total);

            // Initialize the header area to 0.
            memset<address_space>(m_buffer, 0, header_size);

            m_sb_state_array = reinterpret_cast<uint32_t*>(m_buffer);

            // Initialize each superblock's state and the Hint array for each block size.
            for (int32_t i = 0; i < number_block_sizes; ++i)
            {
                uint32_t block_size_lg2          = i + m_min_block_size_lg2;
                uint32_t block_count_lg2         = m_sb_size_lg2 - block_size_lg2;
                uint32_t block_state             = block_count_lg2 << concurrent_bitset::state_shift;
                uint32_t hint_begin              = m_hint_offset + i * s_hint_per_block_size;
                int32_t  jbeg                    = (i * m_sb_count) / number_block_sizes;
                int32_t  jend                    = ((i + 1) * m_sb_count) / number_block_sizes;
                m_sb_state_array[hint_begin]     = static_cast<uint32_t>(jbeg);
                m_sb_state_array[hint_begin + 1] = static_cast<uint32_t>(jbeg);
                for (int32_t j = jbeg; j < jend; ++j)
                {
                    m_sb_state_array[j * m_sb_state_size] = block_state;
                }
            }
        }

        //--------------------------------------------------------------------------
        //  Allocate a block of memory that is at least 'alloc_size'
        //
        //  The block of memory is aligned to the minimum block size,
        //  currently is 64 bytes.
        //
        //--------------------------------------------------------------------------
        blk_t allocate(size_t alloc_size) noexcept
        {
            if (alloc_size > (1UL << m_max_block_size_lg2))
            {
                abort("enda MemoryPool allocation request exceeded specified maximum allocation size");
            }

            if (alloc_size == 0)
            {
                return blk_t {nullptr, 0};
            }

            void*          p              = nullptr;
            const uint32_t block_size_lg2 = get_block_size_lg2(static_cast<uint32_t>(alloc_size));

            // Allocation will fit within a superblock that has block sizes ( 1 << block_size_lg2 )

            const uint32_t block_count_lg2 = m_sb_size_lg2 - block_size_lg2;
            const uint32_t block_state     = block_count_lg2 << concurrent_bitset::state_shift;
            const uint32_t block_count     = 1u << block_count_lg2;

            // Superblock hints for this block size:
            //   hint_sb_id_ptr[0] is the dynamically changing hint
            //   hint_sb_id_ptr[1] is the static start point

            // Get the Hint pointer for the corresponding block size.
            const int32_t   block_size_index = block_size_lg2 - m_min_block_size_lg2;
            uint32_t* const hint_sb_id_ptr   = m_sb_state_array + m_hint_offset + block_size_index * s_hint_per_block_size;
            const int32_t   sb_id_begin      = static_cast<const int32_t>(hint_sb_id_ptr[1]);

            uint32_t  block_id_hint  = static_cast<uint32_t>(clock_tic());
            uint32_t  sb_state       = block_state;
            int32_t   sb_id          = -1;
            uint32_t* sb_state_array = nullptr;

            int32_t attempt_limit = 1;

            while (attempt_limit > 0)
            {
                int32_t hint_sb_id = -1;

                if (sb_id < 0)
                {
                    // No superblock specified, try the hint for this block size
                    sb_id = hint_sb_id = int32_t(*hint_sb_id_ptr);
                    sb_state_array     = m_sb_state_array + (sb_id * m_sb_state_size);
                }

                // If the superblock header matches the expected state, try to acquire a block.
                if (sb_state == (sb_state_array[0] & concurrent_bitset::state_header_mask))
                {
                    const uint32_t count_lg2 = sb_state_array[0] >> concurrent_bitset::state_shift;
                    const uint32_t mask      = (1u << count_lg2) - 1;
                    auto           res       = concurrent_bitset::acquire_bounded_lg2(sb_state_array, count_lg2, block_id_hint & mask);

                    if (res.first >= 0)
                    {
                        const uint32_t size_lg2 = m_sb_size_lg2 - count_lg2;
                        p                       = reinterpret_cast<void*>(reinterpret_cast<char*>(m_sb_state_array + m_data_offset) +
                                                    (static_cast<uint64_t>(sb_id) << m_sb_size_lg2) + (static_cast<uint64_t>(res.first) << size_lg2));

                        break; // Success
                    }
                }

                //------------------------------------------------------------------
                // Arrive here if failed to acquire a block.
                // Must find a new superblock.

                // Start searching at designated index for this block size.
                // Look for superblock that, in preferential order,
                // 1) part-full superblock of this block size
                // 2) empty superblock to claim for this block size
                // 3) part-full superblock of the next larger block size

                sb_state                = block_state;
                sb_id                   = -1;
                bool     update_hint    = false;
                int32_t  sb_id_empty    = -1;
                int32_t  sb_id_large    = -1;
                uint32_t sb_state_large = 0;

                sb_state_array = m_sb_state_array + sb_id_begin * m_sb_state_size;

                for (int32_t i = 0, id = sb_id_begin; i < m_sb_count; ++i)
                {
                    // Query state of the candidate superblock.
                    // Note that the state may change at any moment
                    // as concurrent allocations and deallocations occur.

                    const uint32_t full_state = *sb_state_array;
                    const uint32_t used       = full_state & concurrent_bitset::state_used_mask;
                    const uint32_t state      = full_state & concurrent_bitset::state_header_mask;

                    if (state == block_state)
                    {
                        // Superblock is assigned to this block size

                        if (used < block_count)
                        {
                            sb_id       = id;
                            update_hint = (used + 1 < block_count);
                            break;
                        }
                    }
                    else if (used == 0)
                    {
                        // Superblock is empty

                        if (sb_id_empty == -1)
                        {
                            // Superblock is not assigned to this block size
                            // and is the first empty superblock encountered.
                            // Save this id to use if a partfull superblock is not found.

                            sb_id_empty = id;
                        }
                    }
                    else if ((-1 == sb_id_empty /* have not found an empty */) && (-1 == sb_id_large /* have not found a larger */) &&
                             (state < block_state /* a larger block */) &&
                             // is not full:
                             (used < (1u << (state >> concurrent_bitset::state_shift))))
                    {
                        //  First superblock encountered that is
                        //  larger than this block size and
                        //  has room for an allocation.
                        //  Save this id to use of partfull or empty superblock not found
                        sb_id_large    = id;
                        sb_state_large = state;
                    }

                    // Iterate around the superblock array:

                    if (++id < m_sb_count)
                    {
                        sb_state_array += m_sb_state_size;
                    }
                    else
                    {
                        id             = 0;
                        sb_state_array = m_sb_state_array;
                    }
                }

                if (sb_id < 0)
                {
                    // Did not find a partfull superblock for this block size.

                    if (sb_id_empty >= 0)
                    {
                        // Found first empty superblock following designated superblock
                        // Attempt to claim it for this block size.
                        // If the claim fails assume that another thread claimed it
                        // for this block size and try to use it anyway,
                        // but do not update hint.

                        sb_id          = sb_id_empty;
                        sb_state_array = m_sb_state_array + (sb_id * m_sb_state_size);

                        // If successfully changed assignment of empty superblock 'sb_id'
                        // to this block_size then update the hint.

                        uint32_t state_empty = (*sb_state_array) & concurrent_bitset::state_header_mask;

                        // If this thread claims the empty block then update the hint
                        std::atomic_ref<uint32_t> atomic_state(*const_cast<uint32_t*>(sb_state_array));
                        update_hint = atomic_state.compare_exchange_strong(state_empty, block_state);
                    }
                    else if (sb_id_large >= 0)
                    {
                        // Found a larger superblock with space available
                        sb_id          = sb_id_large;
                        sb_state       = sb_state_large;
                        sb_state_array = m_sb_state_array + (sb_id * m_sb_state_size);
                    }
                    else
                    {
                        // Did not find a potentially usable superblock
                        --attempt_limit;
                    }
                }

                if (update_hint)
                {
                    std::atomic_ref<uint32_t> atomic_hint(*(const_cast<uint32_t*>(hint_sb_id_ptr)));
                    uint32_t                  expected_hint_sb_id = uint32_t(hint_sb_id);
                    atomic_hint.compare_exchange_strong(expected_hint_sb_id, uint32_t(sb_id));
                }
            } // end allocation attempt loop

            return blk_t {(char*)p, alloc_size};
        }

        blk_t allocate_zero(size_t alloc_size) noexcept
        {
            blk_t b = allocate(alloc_size);
            memset<address_space>((void*)b.ptr, 0, alloc_size);
            return b;
        }

        //--------------------------------------------------------------------------
        // deallocate: Return an allocated block of memory to the pool.
        // Requires: b is return value from allocate( alloc_size );
        //--------------------------------------------------------------------------
        inline void deallocate(blk_t b) noexcept
        {
            if (nullptr == b.ptr)
            {
                return;
            }

            // Determine which superblock and block
            const ptrdiff_t d = reinterpret_cast<char*>(b.ptr) - reinterpret_cast<char*>(m_sb_state_array + m_data_offset);
            if (d < 0 || static_cast<size_t>(d) >= (static_cast<size_t>(m_sb_count) << m_sb_size_lg2))
            {
                abort("Pointer out of memory pool bounds.");
            }

            // Verify contained within the memory pool's superblocks:
            const int ok_contains = (0 <= d) && (size_t(d) < (size_t(m_sb_count) << m_sb_size_lg2));

            int ok_block_aligned = 0;
            int ok_dealloc_once  = 0;

            if (ok_contains)
            {
                const int sb_id = d >> m_sb_size_lg2;

                // State array for the superblock.
                uint32_t* const sb_state_array = m_sb_state_array + (sb_id * m_sb_state_size);

                const uint32_t block_state    = (*sb_state_array) & concurrent_bitset::state_header_mask;
                const uint32_t block_size_lg2 = m_sb_size_lg2 - (block_state >> concurrent_bitset::state_shift);

                ok_block_aligned = 0 == (d & ((1UL << block_size_lg2) - 1));

                if (ok_block_aligned)
                {
                    // Map address to block's bit
                    // mask into superblock and then shift down for block index

                    const uint32_t bit = (d & (ptrdiff_t(1LU << m_sb_size_lg2) - 1)) >> block_size_lg2;

                    const int result = concurrent_bitset::release(sb_state_array, bit);

                    ok_dealloc_once = 0 <= result;
                }
            }

            if (!ok_contains || !ok_block_aligned || !ok_dealloc_once)
            {
                abort("MemoryPool::deallocate given erroneous pointer");
            }
        }

        // Returns the total capacity (in bytes) of the memory pool.
        inline size_t capacity() const noexcept { return static_cast<size_t>(m_sb_count) << m_sb_size_lg2; }

        // Returns the minimum block size in bytes.
        inline size_t min_block_size() const noexcept { return 1UL << m_min_block_size_lg2; }

        // Returns the maximum block size in bytes.
        inline size_t max_block_size() const noexcept { return 1UL << m_max_block_size_lg2; }

        // Returns the number of superblocks.
        inline int number_of_superblocks() const noexcept { return m_sb_count; }

        inline void superblock_state(int sb_id, int& block_size, int& block_count_capacity, int& block_count_used) const noexcept
        {
            block_size           = 0;
            block_count_capacity = 0;
            block_count_used     = 0;

            const uint32_t state = ((uint32_t volatile*)m_sb_state_array)[sb_id * m_sb_state_size];

            const uint32_t block_count_lg2 = state >> concurrent_bitset::state_shift;
            const uint32_t block_used      = state & concurrent_bitset::state_used_mask;

            block_size           = 1LU << (m_sb_size_lg2 - block_count_lg2);
            block_count_capacity = 1LU << block_count_lg2;
            block_count_used     = block_used;
        }

        // Copy the usage statistics into the provided structure.
        void get_usage_statistics(usage_statistics& stats) const
        {
            stats.superblock_bytes     = 1UL << m_sb_size_lg2;
            stats.max_block_bytes      = 1UL << m_max_block_size_lg2;
            stats.min_block_bytes      = 1UL << m_min_block_size_lg2;
            stats.capacity_bytes       = stats.superblock_bytes * m_sb_count;
            stats.capacity_superblocks = m_sb_count;
            stats.consumed_superblocks = 0;
            stats.consumed_blocks      = 0;
            stats.consumed_bytes       = 0;
            stats.reserved_blocks      = 0;
            stats.reserved_bytes       = 0;

            // Traverse each superblock to accumulate usage and reserved information.
            for (int32_t i = 0; i < m_sb_count; ++i)
            {
                uint32_t* sb_state        = m_sb_state_array + i * m_sb_state_size;
                uint32_t  state           = sb_state[0];
                uint32_t  block_count_lg2 = state >> concurrent_bitset::state_shift;
                if (block_count_lg2)
                {
                    uint32_t block_count = 1u << block_count_lg2;
                    uint32_t block_size  = 1u << (m_sb_size_lg2 - block_count_lg2);
                    uint32_t block_used  = state & concurrent_bitset::state_used_mask;
                    stats.consumed_superblocks++;
                    stats.consumed_blocks += block_used;
                    stats.consumed_bytes += static_cast<size_t>(block_used) * block_size;
                    stats.reserved_blocks += block_count - block_used;
                    stats.reserved_bytes += static_cast<size_t>(block_count - block_used) * block_size;
                }
            }
        }

#ifndef NDEBUG
        // Print the memory pool state to the provided output stream.
        void print_state(std::ostream& os) const
        {
            os << "MemoryPool state:\n";
            os << "    Pool size (bytes):" << (size_t(m_sb_count) << m_sb_size_lg2) << "\n";
            os << "    Superblock count: " << m_sb_count << "\n";
            os << "    Superblock size (bytes): " << (1UL << m_sb_size_lg2) << "\n";

            for (int32_t i = 0; i < m_sb_count; ++i)
            {
                uint32_t* sb_state_ptr = m_sb_state_array + i * m_sb_state_size;

                if (*sb_state_ptr)
                {
                    const uint32_t block_count_lg2 = (*sb_state_ptr) & concurrent_bitset::state_header_mask;
                    const uint32_t block_size_lg2  = m_sb_size_lg2 - block_count_lg2;
                    const uint32_t block_count     = 1u << block_count_lg2;
                    const uint32_t block_used      = (*sb_state_ptr) & concurrent_bitset::state_used_mask;

                    os << "    Superblock[ " << i << " / " << m_sb_count << " ]:"
                       << " block_size(" << (1 << block_size_lg2) << ")"
                       << " block_count(" << block_used << " / " << block_count << ")" << "\n";
                }
            }
        }
#endif

    private:
        // Compute the block size exponent for n (must not be lower than m_min_block_size_lg2).
        inline uint32_t get_block_size_lg2(uint32_t n) const noexcept
        {
            uint32_t exp = integral_power_of_two_that_contains(n);
            return exp < m_min_block_size_lg2 ? m_min_block_size_lg2 : exp;
        }

        /* Verify size constraints:
         *   min_block_alloc_size <= max_block_alloc_size
         *   max_block_alloc_size <= min_superblock_size
         *   min_superblock_size  <= max_superblock_size
         *   min_superblock_size  <= min_total_alloc_size
         *   min_superblock_size  <= min_block_alloc_size *
         *                           max_block_per_superblock
         */
        void memory_pool_bounds_verification(size_t min_block_alloc_size,
                                             size_t max_block_alloc_size,
                                             size_t min_superblock_size,
                                             size_t max_superblock_size,
                                             size_t max_block_per_superblock,
                                             size_t min_total_alloc_size)
        {
            const size_t max_superblock = min_block_alloc_size * max_block_per_superblock;

            if ((size_t(max_superblock_size) < min_superblock_size) || (min_total_alloc_size < min_superblock_size) || (max_superblock < min_superblock_size) ||
                (min_superblock_size < max_block_alloc_size) || (max_block_alloc_size < min_block_alloc_size))
            {
                std::ostringstream msg;

                msg << "MemoryPool size constraint violation";

                if (size_t(max_superblock_size) < min_superblock_size)
                {
                    msg << " : max_superblock_size(" << max_superblock_size << ") < min_superblock_size(" << min_superblock_size << ")";
                }

                if (min_total_alloc_size < min_superblock_size)
                {
                    msg << " : min_total_alloc_size(" << min_total_alloc_size << ") < min_superblock_size(" << min_superblock_size << ")";
                }

                if (max_superblock < min_superblock_size)
                {
                    msg << " : max_superblock(" << max_superblock << ") < min_superblock_size(" << min_superblock_size << ")";
                }

                if (min_superblock_size < max_block_alloc_size)
                {
                    msg << " : min_superblock_size(" << min_superblock_size << ") < max_block_alloc_size(" << max_block_alloc_size << ")";
                }

                if (max_block_alloc_size < min_block_alloc_size)
                {
                    msg << " : max_block_alloc_size(" << max_block_alloc_size << ") < min_block_alloc_size(" << min_block_alloc_size << ")";
                }

                ENDA_RUNTIME_ERROR << msg.str();
            }
        }

    private:
        char*     m_buffer             = nullptr; // Underlying memory buffer
        uint32_t* m_sb_state_array     = nullptr; // Pointer to the state array (header)
        int32_t   m_sb_state_size      = 0;       // Number of uint32_t per superblock state
        uint32_t  m_sb_size_lg2        = 0;       // Exponent for superblock size (bytes = 1 << m_sb_size_lg2)
        uint32_t  m_max_block_size_lg2 = 0;       // Exponent for maximum block size
        uint32_t  m_min_block_size_lg2 = 0;       // Exponent for minimum block size
        int32_t   m_sb_count           = 0;       // Number of superblocks
        int32_t   m_hint_offset        = 0;       // Offset to the Hint array in the state area (in uint32_t)
        int32_t   m_data_offset        = 0;       // Offset to the data region in the buffer (in uint32_t)
        int32_t   m_unused_padding     = 0;       // Unused padding (for alignment)

        // The maximum size of a superblock.
        static constexpr uint32_t s_max_superblock_size = 1UL << 31; // 2GB
        // Number of uint32_t elements used per block size in the Hint array.
        static constexpr uint32_t s_hint_per_block_size = 2;
    };

    /**
     * @brief Custom allocator that dispatches memory allocation to one of two allocators based on the size of the memory
     * block to be allocated.
     *
     * @note Only works if both allocators have the same enda::mem::AddressSpace.
     *
     * @tparam Threshold Size in bytes that determines which allocator to use.
     * @tparam A enda::mem::Allocator for small memory blocks.
     * @tparam B enda::mem::Allocator for big memory blocks.
     */
    template<size_t Threshold, Allocator A, Allocator B>
    class segregator : public immovable<segregator<Threshold, A, B>>
    {
        // Allocator for small memory blocks.
        A small;

        // Allocator for big memory blocks.
        B big;

    public:
        static_assert(A::address_space == B::address_space);

        // enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /**
         * @brief Allocate memory using the small allocator if the size is less than or equal to the `Threshold`, otherwise
         * use the big allocator.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        inline blk_t allocate(size_t alloc_size) noexcept { return alloc_size <= Threshold ? small.allocate(alloc_size) : big.allocate(alloc_size); }

        /**
         * @brief Allocate memory and set the memory to zero using the small allocator if the size is less than or equal to
         * the `Threshold`, otherwise use the big allocator.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        inline blk_t allocate_zero(size_t alloc_size) noexcept
        {
            return alloc_size <= Threshold ? small.allocate_zero(alloc_size) : big.allocate_zero(alloc_size);
        }

        /**
         * @brief Deallocate memory using the small allocator if the size is less than or equal to the `Threshold`,
         * otherwise use the big allocator.
         *
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        inline void deallocate(blk_t b) noexcept { return b.s <= Threshold ? small.deallocate(b) : big.deallocate(b); }
    };

    /**
     * @brief Wrap an allocator to check for memory leaks.
     *
     * @details It simply keeps track of the memory currently being used by the allocator, i.e. the total memory allocated
     * minus the total memory deallocated, which should never be smaller than zero and should be exactly zero when the
     * allocator is destroyed.
     *
     * @tparam A enda::mem::Allocator type to wrap.
     */
    template<Allocator A>
    class leak_check : A
    {
        // Total memory used by the allocator.
        std::atomic<long> memory_used = 0;

    public:
        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /**
         * @brief Destructor that checks for memory leaks.
         * @details In debug mode, it aborts the program if there is a memory leak.
         */
        ~leak_check()
        {
#ifndef NDEBUG
            long mem = memory_used.load(std::memory_order_acquire);

            if (mem != 0)
            {
                std::ostringstream info;
                info << "Memory leak in allocator: " << memory_used << " bytes leaked";
                abort(info.str().c_str());
            }
#endif
        }

        /**
         * @brief Allocate memory and update the total memory used.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        inline blk_t allocate(size_t alloc_size) noexcept
        {
            blk_t b = A::allocate(alloc_size);
            memory_used.fetch_add(static_cast<long>(b.s), std::memory_order_relaxed);
            return b;
        }

        /**
         * @brief Allocate memory, set it to zero and update the total memory used.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        inline blk_t allocate_zero(size_t alloc_size) noexcept
        {
            blk_t b = A::allocate_zero(alloc_size);
            memory_used.fetch_add(static_cast<long>(b.s), std::memory_order_relaxed);
            return b;
        }

        /**
         * @brief Deallocate memory and update the total memory used.
         * @details In debug mode, it aborts the program if the total memory used is smaller than zero.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        inline void deallocate(blk_t b) noexcept
        {
            memory_used.fetch_sub(static_cast<long>(b.s), std::memory_order_relaxed);
            long mem = memory_used.load(std::memory_order_acquire);
#ifndef NDEBUG
            if (mem < 0)
            {
                std::ostringstream info;
                info << "Memory used by allocator < 0: Memory block to be deleted: b.s = " << b.s << ", b.ptr = " << (void*)b.ptr;
                abort(info.str().c_str());
            }
#endif
            A::deallocate(b);
        }

        /**
         * @brief Check if the base allocator is empty.
         * @return True if no memory is currently being used.
         */
        [[nodiscard]] bool empty() const { return (memory_used.load(std::memory_order_acquire) == 0); }

        /**
         * @brief Get the total memory used by the base allocator.
         * @return The size of the memory which has been allocated and not yet deallocated.
         */
        [[nodiscard]] long get_memory_used() const noexcept { return memory_used.load(std::memory_order_acquire); }
    };

    /**
     * @brief Wrap an allocator to gather statistics about memory allocation.
     *
     * @details It gathers a histogram of the different allocation sizes. The histogram is a std::vector of size 65, where
     * element \f$ i \in \{0,...,63\} \f$ contains the number of allocations with a size in the range
     * \f$ [2^{64-i-1}, 2^{64-i}) \f$ and the last element contains the number of allocations of size zero.
     *
     * @tparam A enda::mem::Allocator type to wrap.
     */
    template<Allocator A>
    class stats : A
    {
        // Histogram of the allocation sizes.
        std::array<std::atomic<uint64_t>, 65> hist;

    public:
        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /// Destructor that outputs the statistics about the memory allocation in debug mode.
        ~stats() { print_histogram(std::cerr); }

        /**
         * @brief Allocate memory and update the histogram.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(uint64_t alloc_size)
        {
            if (alloc_size == 0)
            {
                hist.back().fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                hist[std::countl_zero(alloc_size)].fetch_add(1, std::memory_order_relaxed);
            }
            return A::allocate(alloc_size);
        }

        /**
         * @brief Allocate memory, set it to zero and update the histogram.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(uint64_t alloc_size)
        {
            if (alloc_size == 0)
            {
                hist.back().fetch_add(1, std::memory_order_relaxed);
            }
            else
            {
                hist[std::countl_zero(alloc_size)].fetch_add(1, std::memory_order_relaxed);
            }
            return A::allocate_zero(alloc_size);
        }

        /**
         * @brief Deallocate memory.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept { A::deallocate(b); }

        /**
         * @brief Print the histogram to a std::ostream.
         * @param os std::ostream object to print to.
         */
        void print_histogram(std::ostream& os) const
        {
            os << "Allocation size histogram :\n";
            os << "[0, 2^0): " << hist.back().load(std::memory_order_relaxed) << "\n";
            for (int i = 0; i < 64; ++i)
            {
                os << "[2^" << i << ", 2^" << (i + 1) << "): " << hist[63 - i].load(std::memory_order_relaxed) << "\n";
            }
        }
    };

} // namespace enda::mem
