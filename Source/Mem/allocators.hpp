#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/Malloc.hpp"
#include "Mem/Memset.hpp"

#ifndef NDEBUG
    #include <iostream>
#endif

#if defined(__has_feature)
    #if __has_feature(address_sanitizer)
        #include <sanitizer/asan_interface.h>
        #define ENDA_USE_ASAN
    #endif
#endif

namespace enda::mem
{
    /// Memory block consisting of a pointer and its size.
    struct blk_t
    {
        /// Pointer to the memory block.
        char* ptr = nullptr;

        /// Size of the memory block in bytes.
        size_t s = 0;
    };

    /**
     * @brief Custom allocator that uses enda::mem::malloc to allocate memory.
     * @tparam AdrSp enda::mem::AddressSpace in which the memory is allocated.
     */
    template<AddressSpace AdrSp = Host>
    class mallocator
    {
    public:
        /// Default constructor.
        mallocator() = default;

        /// Deleted copy constructor.
        mallocator(mallocator const&) = delete;

        /// Default move constructor.
        mallocator(mallocator&&) = default;

        /// Deleted copy assignment operator.
        mallocator& operator=(mallocator const&) = delete;

        /// Default move assignment operator.
        mallocator& operator=(mallocator&&) = default;

        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = AdrSp;

        /**
         * @brief Allocate memory using enda::mem::malloc.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        static blk_t allocate(size_t s) noexcept { return {(char*)malloc<AdrSp>(s), s}; }

        /**
         * @brief Allocate memory and set it to zero.
         *
         * @details The behavior depends on the address space:
         * - It uses std::calloc for `Host` enda::mem::AddressSpace.
         * - Otherwise it uses enda::mem::malloc and enda::mem::memset.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        static blk_t allocate_zero(size_t s) noexcept
        {
            if constexpr (AdrSp == mem::Host)
            {
                return {(char*)std::calloc(s, 1 /* byte */), s}; // NOLINT (C-style cast is fine here)
            }
            else
            {
                char* ptr = (char*)malloc<AdrSp>(s);
                memset<AdrSp>(ptr, 0, s);
                return {ptr, s};
            }
        }

        /**
         * @brief Deallocate memory using enda::mem::free.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        static void deallocate(blk_t b) noexcept { free<AdrSp>((void*)b.ptr); }
    };

    /**
     * @brief Compute the smallest exponent 'exp' such that 2^exp >= n.
     * @param n enda::mem::blk_t memory block to deallocate.
     */
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

    //==============================================================================
    // MemoryPool
    //
    // The MemoryPool divides a large memory block into several "superblocks", each
    // of which is subdivided into equally sized "blocks". Each superblock maintains
    // a concurrent bitset state to mark whether each block is allocated.
    //==============================================================================
    class MemoryPool
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

        // Returns the total capacity (in bytes) of the memory pool.
        constexpr size_t capacity() const noexcept { return static_cast<size_t>(m_sb_count) << m_sb_size_lg2; }

        // Returns the minimum block size in bytes.
        constexpr size_t min_block_size() const noexcept { return 1UL << m_min_block_size_lg2; }

        // Returns the maximum block size in bytes.
        constexpr size_t max_block_size() const noexcept { return 1UL << m_max_block_size_lg2; }

        // Returns the number of superblocks.
        constexpr int number_of_superblocks() const noexcept { return m_sb_count; }

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
                uint32_t  block_count_lg2 = state >> m_state_shift;
                if (block_count_lg2)
                {
                    uint32_t block_count = 1u << block_count_lg2;
                    uint32_t block_size  = 1u << (m_sb_size_lg2 - block_count_lg2);
                    uint32_t block_used  = state & m_state_used_mask;
                    stats.consumed_superblocks++;
                    stats.consumed_blocks += block_used;
                    stats.consumed_bytes += static_cast<size_t>(block_used) * block_size;
                    stats.reserved_blocks += block_count - block_used;
                    stats.reserved_bytes += static_cast<size_t>(block_count - block_used) * block_size;
                }
            }
        }

        // Print the memory pool state to the provided output stream.
        void print_state(std::ostream& os) const
        {
            os << "MemoryPool state:\n";
            os << "Superblock count: " << m_sb_count << "\n";
            os << "Superblock size (bytes): " << (1UL << m_sb_size_lg2) << "\n";
            for (int32_t i = 0; i < m_sb_count; ++i)
            {
                uint32_t* sb_state = m_sb_state_array + i * m_sb_state_size;
                os << "Superblock " << i << " state: " << sb_state[0] << "\n";
            }
        }

        //--------------------------------------------------------------------------
        // Constructor:
        //   min_total_alloc_size: Minimum total bytes that the memory pool must provide.
        //   min_block_alloc_size: Minimum block allocation size in bytes (default 64 bytes).
        //   max_block_alloc_size: Maximum block allocation size in bytes (default 4KB).
        //   min_superblock_size: Minimum superblock size in bytes (default 1MB).
        //--------------------------------------------------------------------------
        MemoryPool(size_t min_total_alloc_size, size_t min_block_alloc_size = 0, size_t max_block_alloc_size = 0, size_t min_superblock_size = 0)
        {
            // Alignment requirements: align to 8 uint32_t elements.
            constexpr uint32_t int_align_lg2               = 3;
            constexpr uint32_t int_align_mask              = (1u << int_align_lg2) - 1;
            constexpr uint32_t default_min_block_size      = 1u << 6;  // 64 bytes
            constexpr uint32_t default_max_block_size      = 1u << 12; // 4096 bytes
            constexpr uint32_t default_min_superblock_size = 1u << 20; // 1MB

            // Set default values if not provided.
            if (min_block_alloc_size == 0)
            {
                min_superblock_size  = std::min(default_min_superblock_size, static_cast<uint32_t>(min_total_alloc_size));
                min_block_alloc_size = std::min(default_min_block_size, min_superblock_size);
                max_block_alloc_size = std::min(default_max_block_size, min_superblock_size);
            }
            else if (min_superblock_size == 0)
            {
                size_t max_superblock = min_block_alloc_size * m_max_block_per_superblock;
                min_superblock_size   = std::min(max_superblock, std::min((size_t)default_min_superblock_size, min_total_alloc_size));
            }
            if (max_block_alloc_size == 0)
            {
                max_block_alloc_size = min_superblock_size;
            }

            // Compute the exponents for the various sizes (i.e., power-of-two representation).
            m_min_block_size_lg2 = integral_power_of_two_that_contains(min_block_alloc_size);
            m_max_block_size_lg2 = integral_power_of_two_that_contains(max_block_alloc_size);
            m_sb_size_lg2        = integral_power_of_two_that_contains(min_superblock_size);

            // Compute the number of superblocks needed to cover min_total_alloc_size.
            uint64_t sb_size = 1UL << m_sb_size_lg2;
            m_sb_count       = static_cast<int32_t>((min_total_alloc_size + sb_size - 1) >> m_sb_size_lg2);

            // Compute the size of each superblock's state area using concurrent_bitset::buffer_bound_lg2.
            uint32_t max_block_count_lg2 = m_sb_size_lg2 - m_min_block_size_lg2;
            uint32_t cb_size             = concurrent_bitset::buffer_bound_lg2(max_block_count_lg2);
            m_sb_state_size              = (cb_size + int_align_mask) & ~int_align_mask;

            // Compute the total state area size (in uint32_t elements).
            uint32_t all_sb_state_size = (m_sb_count * m_sb_state_size + int_align_mask) & ~int_align_mask;

            // Compute the number of different block sizes.
            int32_t number_block_sizes = 1 + m_max_block_size_lg2 - m_min_block_size_lg2;
            // Hint array size: one uint32_t per block size, aligned.
            int32_t block_size_array_size = (number_block_sizes + int_align_mask) & ~int_align_mask;

            m_hint_offset = all_sb_state_size;
            m_data_offset = m_hint_offset + block_size_array_size * m_hint_per_block_size;

            // Compute total allocation size: header area + superblock data area.
            size_t header_size      = m_data_offset * sizeof(uint32_t);
            size_t alloc_size_total = header_size + (static_cast<size_t>(m_sb_count) << m_sb_size_lg2);

            // Allocate memory using a vector as the underlying buffer.
            m_buffer.resize(alloc_size_total, 0);
            m_sb_state_array = reinterpret_cast<uint32_t*>(m_buffer.data());

            // Initialize the header area to 0.
            std::fill(m_sb_state_array, m_sb_state_array + m_data_offset, 0);

            // Initialize each superblock's state and the Hint array for each block size.
            for (int32_t i = 0; i < number_block_sizes; ++i)
            {
                uint32_t block_size_lg2          = i + m_min_block_size_lg2;
                uint32_t block_count_lg2         = m_sb_size_lg2 - block_size_lg2;
                uint32_t block_state             = block_count_lg2 << m_state_shift;
                uint32_t hint_begin              = m_hint_offset + i * m_hint_per_block_size;
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
        // allocate: Allocate a memory block of at least alloc_size bytes.
        // If allocation fails, attempt up to attempt_limit times.
        //--------------------------------------------------------------------------
        void* allocate(size_t alloc_size, int32_t attempt_limit = 1)
        {
            if (alloc_size > (1UL << m_max_block_size_lg2))
            {
                std::cerr << "Allocation request exceeds maximum block size.\n";
                std::abort();
            }
            if (alloc_size == 0)
                return nullptr;

            void*    p               = nullptr;
            uint32_t block_size_lg2  = std::max(get_block_size_lg2(static_cast<uint32_t>(alloc_size)), m_min_block_size_lg2);
            uint32_t block_count_lg2 = m_sb_size_lg2 - block_size_lg2;
            uint32_t block_state     = block_count_lg2 << m_state_shift;
            uint32_t block_count     = 1u << block_count_lg2;

            // Get the Hint pointer for the corresponding block size.
            int32_t   block_size_index = block_size_lg2 - m_min_block_size_lg2;
            uint32_t* hint_sb_id_ptr   = m_sb_state_array + m_hint_offset + block_size_index * m_hint_per_block_size;
            int32_t   sb_id_begin      = static_cast<int32_t>(hint_sb_id_ptr[1]);

            // Use current steady_clock time as a pseudo-random hint.
            uint32_t  block_id_hint     = static_cast<uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
            uint32_t  sb_state_expected = block_state;
            int32_t   sb_id             = -1;
            uint32_t* sb_state_array    = nullptr;

            while (attempt_limit > 0)
            {
                if (sb_id < 0)
                {
                    sb_id          = static_cast<int32_t>(hint_sb_id_ptr[0]);
                    sb_state_array = m_sb_state_array + sb_id * m_sb_state_size;
                }
                // If the superblock header matches the expected state, try to acquire a block.
                if ((sb_state_array[0] & m_state_header_mask) == sb_state_expected)
                {
                    uint32_t count_lg2 = sb_state_array[0] >> m_state_shift;
                    uint32_t mask      = (1u << count_lg2) - 1;
                    auto     res       = concurrent_bitset::acquire_bounded_lg2(sb_state_array, count_lg2, block_id_hint & mask, sb_state_array[0]);
                    if (res.first >= 0)
                    { // Successful acquisition
                        uint32_t size_lg2 = m_sb_size_lg2 - count_lg2;
                        p                 = reinterpret_cast<void*>(reinterpret_cast<char*>(m_sb_state_array + m_data_offset) +
                                                    (static_cast<uint64_t>(sb_id) << m_sb_size_lg2) + (static_cast<uint64_t>(res.first) << size_lg2));
                        break;
                    }
                }
                // Simplified: if allocation fails, decrement attempt count (actual implementation would search other superblocks).
                sb_id = -1;
                --attempt_limit;
            }
            return p;
        }

        //--------------------------------------------------------------------------
        // deallocate: Return an allocated memory block back to the pool.
        // Parameter p must be a pointer previously returned by allocate.
        //--------------------------------------------------------------------------
        void deallocate(void* p, size_t /* alloc_size */)
        {
            if (p == nullptr)
                return;

            // Compute the offset 'd' from the start of the data region.
            ptrdiff_t d = reinterpret_cast<char*>(p) - reinterpret_cast<char*>(m_sb_state_array + m_data_offset);
            if (d < 0 || static_cast<size_t>(d) >= (static_cast<size_t>(m_sb_count) << m_sb_size_lg2))
            {
                std::cerr << "Pointer out of memory pool bounds.\n";
                std::abort();
            }
            int32_t   sb_id          = static_cast<int32_t>(d) >> m_sb_size_lg2;
            uint32_t* sb_state_array = m_sb_state_array + sb_id * m_sb_state_size;
            uint32_t  state          = sb_state_array[0] & m_state_header_mask;
            uint32_t  block_size_lg2 = m_sb_size_lg2 - (state >> m_state_shift);
            // Check if the address is block-aligned.
            if (d & ((1UL << block_size_lg2) - 1))
            {
                std::cerr << "Pointer not block aligned.\n";
                std::abort();
            }
            uint32_t bit    = (static_cast<uint32_t>(d) & ((1UL << m_sb_size_lg2) - 1)) >> block_size_lg2;
            int      result = concurrent_bitset::release(sb_state_array, bit, state);
            if (result < 0)
            {
                std::cerr << "Error during deallocation.\n";
                std::abort();
            }
        }

    private:
        // Compute the block size exponent for n (must not be lower than m_min_block_size_lg2).
        uint32_t get_block_size_lg2(uint32_t n) const noexcept
        {
            uint32_t exp = integral_power_of_two_that_contains(n);
            return exp < m_min_block_size_lg2 ? m_min_block_size_lg2 : exp;
        }

        //--------------------------------------------------------------------------
        // Member variables:
        // 1. m_buffer: Underlying memory buffer (using std::vector<uint8_t>).
        // 2. m_sb_state_array: Pointer to the state array (header region) within the buffer.
        // 3. m_sb_state_size: Number of uint32_t elements per superblock state area.
        // 4. m_sb_size_lg2: Exponent for the superblock size (bytes = 1 << m_sb_size_lg2).
        // 5. m_max_block_size_lg2, m_min_block_size_lg2: Exponents for maximum and minimum block sizes.
        // 6. m_sb_count: Number of superblocks.
        // 7. m_hint_offset: Offset (in uint32_t elements) to the Hint array within the state area.
        // 8. m_data_offset: Offset (in uint32_t elements) to the data region within the buffer.
        //--------------------------------------------------------------------------
        std::vector<uint8_t> m_buffer;                       // Underlying memory buffer
        uint32_t*            m_sb_state_array     = nullptr; // Pointer to the state array (header)
        int32_t              m_sb_state_size      = 0;       // Number of uint32_t per superblock state
        uint32_t             m_sb_size_lg2        = 0;       // Exponent for superblock size (bytes = 1 << m_sb_size_lg2)
        uint32_t             m_max_block_size_lg2 = 0;       // Exponent for maximum block size
        uint32_t             m_min_block_size_lg2 = 0;       // Exponent for minimum block size
        int32_t              m_sb_count           = 0;       // Number of superblocks
        int32_t              m_hint_offset        = 0;       // Offset to the Hint array in the state area (in uint32_t)
        int32_t              m_data_offset        = 0;       // Offset to the data region in the buffer (in uint32_t)
        int32_t              m_unused_padding     = 0;       // Unused padding (for alignment)

        // Constants for state header shift and mask.
        static constexpr uint32_t m_state_shift     = concurrent_bitset::state_shift;
        static constexpr uint32_t m_state_used_mask = concurrent_bitset::state_used_mask;
        // Maximum number of blocks per superblock as defined by the concurrent bitset.
        static constexpr int32_t m_max_block_per_superblock = concurrent_bitset::max_bit_count;
        // Number of uint32_t elements used per block size in the Hint array.
        static constexpr int32_t m_hint_per_block_size = 2;
    };

    /**
     * @brief Custom allocator that allocates a bucket of memory on the heap consisting of 64 chunks.
     *
     * @details The allocator keeps track of which chunks are free using a bitmask. Once all chunks have been allocated,
     * it will call std::abort on any further allocation requests.
     *
     * @note Only works with `Host` enda::mem::AddressSpace.
     *
     * @tparam ChunkSize Size of the chunks in bytes.
     */
    template<int ChunkSize>
    class bucket
    {
        // Unique pointer to handle memory allocation for the bucket.
        std::unique_ptr<char[]> _start = std::make_unique<char[]>(TotalChunkSize); // NOLINT (C-style array is fine here)

        // Pointer to the start of the bucket.
        char* p = _start.get();

        // Bitmask to keep track of which chunks are free.
        uint64_t flags = uint64_t(-1);

    public:
        /// Total size of the bucket in bytes.
        static constexpr int TotalChunkSize = 64 * ChunkSize;

        /// Only `Host` enda::mem::AddressSpace is supported for this allocator.
        static constexpr auto address_space = Host;

#ifdef ENDA_USE_ASAN
        bucket() { __asan_poison_memory_region(p, TotalChunkSize); }
        ~bucket() { __asan_unpoison_memory_region(p, TotalChunkSize); }
#else
        /// Default constructor.
        bucket() = default;
#endif
        /// Deleted copy constructor.
        bucket(bucket const&) = delete;

        /// Default move constructor.
        bucket(bucket&&) = default;

        /// Deleted copy assignment operator.
        bucket& operator=(bucket const&) = delete;

        /// Default move assignment operator.
        bucket& operator=(bucket&&) = default;

        /**
         * @brief Allocate a chunk of memory in the bucket and update the bitmask.
         *
         * @param s Size in bytes of the returned memory block (has to be < `ChunkSize`).
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(size_t s) noexcept
        {
            // check the size and if there is a free chunk, otherwise abort
            if (s > ChunkSize)
                std::abort();
            if (flags == 0)
                std::abort();

            // find the first free chunk
            int pos = __builtin_ctzll(flags);

            // update the bitmask and return the memory block
            flags &= ~(1ull << pos);
            blk_t b {p + static_cast<ptrdiff_t>(pos * ChunkSize), s};
#ifdef ENDA_USE_ASAN
            __asan_unpoison_memory_region(b.ptr, ChunkSize);
#endif
            return b;
        }

        /**
         * @brief Allocate a chunk of memory in the bucket, set it to zero and update the bitmask.
         *
         * @param s Size in bytes of the returned memory block (has to be < `ChunkSize`).
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(size_t s) noexcept
        {
            auto blk = allocate(s);
            std::memset(blk.ptr, 0, s);
            return blk;
        }

        /**
         * @brief Deallocate a chunk of memory from the bucket by simply resetting the bitmask.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept
        {
#ifdef ENDA_USE_ASAN
            __asan_poison_memory_region(b.ptr, ChunkSize);
#endif
            int pos = (b.ptr - p) / ChunkSize;
            flags |= (1ull << pos);
        }

        /**
         * @brief Check if the bucket is full.
         * @return True if there are no free chunks left, i.e. the bitmask is all zeros.
         */
        [[nodiscard]] bool is_full() const noexcept { return flags == 0; }

        /**
         * @brief Check if the bucket is empty.
         * @return True if all chunks are free, i.e. the bitmask is all ones.
         */
        [[nodiscard]] bool empty() const noexcept { return flags == uint64_t(-1); }

        /**
         * @brief Get a pointer to the start of the bucket.
         * @return Pointer to the chunk with the lowest memory address.
         */
        [[nodiscard]] const char* data() const noexcept { return p; }

        /**
         * @brief Get the bitmask of the bucket.
         * @return Bitmask in the form of a `uint64_t`.
         */
        [[nodiscard]] auto mask() const noexcept { return flags; }

        /**
         * @brief Check if a given enda::mem::blk_t memory block is owned by the bucket.
         *
         * @param b enda::mem::blk_t memory block.
         * @return True if the memory block is owned by the bucket.
         */
        [[nodiscard]] bool owns(blk_t b) const noexcept { return b.ptr >= p and b.ptr < p + TotalChunkSize; }
    };

    /**
     * @brief Custom allocator that uses multiple enda::mem::bucket allocators.
     *
     * @details It uses a std::vector of bucket allocators. When all buckets in the vector are full, it simply adds a new
     * one at the end.
     *
     * @note Only works with `Host` enda::mem::AddressSpace.
     *
     * @tparam ChunkSize Size of the chunks in bytes.
     */
    template<int ChunkSize>
    class multi_bucket
    {
        // Alias for the bucket allocator type.
        using b_t = bucket<ChunkSize>;

        // Vector of enda::mem::bucket allocators (ordered in memory).
        std::vector<b_t> bu_vec;

        // Iterator to the current bucket in use.
        typename std::vector<b_t>::iterator bu;

        // Find a bucket with a free chunk, otherwise create a new one and insert it at the correct position.
        [[gnu::noinline]] void find_non_full_bucket()
        {
            bu = std::find_if(bu_vec.begin(), bu_vec.end(), [](auto const& b) { return !b.is_full(); });
            if (bu != bu_vec.end())
                return;
            b_t  b;
            auto pos = std::upper_bound(bu_vec.begin(), bu_vec.end(), b, [](auto const& b1, auto const& b2) { return b1.data() < b2.data(); });
            bu       = bu_vec.insert(pos, std::move(b));
        }

    public:
        /// Only `Host` enda::mem::AddressSpace is supported for this allocator.
        static constexpr auto address_space = Host;

        /// Default constructor.
        multi_bucket() : bu_vec(1), bu(bu_vec.begin()) {}

        /// Deleted copy constructor.
        multi_bucket(multi_bucket const&) = delete;

        /// Default move constructor.
        multi_bucket(multi_bucket&&) = default;

        /// Deleted copy assignment operator.
        multi_bucket& operator=(multi_bucket const&) = delete;

        /// Default move assignment operator.
        multi_bucket& operator=(multi_bucket&&) = default;

        /**
         * @brief Allocate a chunk of memory in the current bucket or find a new one if the current one is full.
         *
         * @param s Size in bytes of the returned memory block (has to be < `ChunkSize`).
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(size_t s) noexcept
        {
            if ((bu == bu_vec.end()) or (bu->is_full()))
                find_non_full_bucket();
            return bu->allocate(s);
        }

        /**
         * @brief Allocate a chunk of memory in the current bucket or find a new one if the current one is full and set it
         * to zero.
         *
         * @param s Size in bytes of the returned memory block (has to be < `ChunkSize`).
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(size_t s) noexcept
        {
            auto blk = allocate(s);
            std::memset(blk.ptr, 0, s);
            return blk;
        }

        /**
         * @brief Deallocate a chunk of memory from the bucket to which it belongs.
         *
         * @details If the bucket is empty after deallocation and it is not the only one, it is removed from the vector of
         * buckets.
         *
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept
        {
            // try the current bucket first
            if (bu != bu_vec.end() and bu->owns(b))
            {
                bu->deallocate(b);
                return;
            }

            // otherwise, find the owning bucket in the vector and deallocate
            bu = std::lower_bound(bu_vec.begin(), bu_vec.end(), b.ptr, [](auto const& b1, auto p) { return b1.data() <= p; });
            --bu;
            EXPECTS_WITH_MESSAGE((bu != bu_vec.end()), "Error in enda::mem::multi_bucket::deallocate: Owning bucket not found");
            EXPECTS_WITH_MESSAGE((bu->owns(b)), "Error in enda::mem::multi_bucket::deallocate: Owning bucket not found");
            bu->deallocate(b);

            // remove bucket the current bucket if it is empty and not the only one
            if (!bu->empty())
                return;
            if (bu_vec.size() <= 1)
                return;
            bu_vec.erase(bu);
            bu = bu_vec.end();
        }

        /**
         * @brief Check if the current allocator is empty.
         * @return True if there is only one bucket in the vector and it is empty.
         */
        [[nodiscard]] bool empty() const noexcept { return bu_vec.size() == 1 && bu_vec[0].empty(); }

        /**
         * @brief Get the bucket vector.
         * @return std::vector with all the bucket allocators currently in use.
         */
        [[nodiscard]] auto const& buckets() const noexcept { return bu_vec; }

        /**
         * @brief Check if a given enda::mem::blk_t memory block is owned by allocator.
         *
         * @param b enda::mem::blk_t memory block.
         * @return True if the memory block is owned by one of the buckets.
         */
        [[nodiscard]] bool owns(blk_t b) const noexcept
        {
            bool res = false;
            for (const auto& mb : bu_vec)
            {
                res = res || mb.owns(b);
            }
            return res;
        }
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
    class segregator
    {
        // Allocator for small memory blocks.
        A small;

        // Allocator for big memory blocks.
        B big;

    public:
        static_assert(A::address_space == B::address_space);

        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /// Default constructor.
        segregator() = default;

        /// Deleted copy constructor.
        segregator(segregator const&) = delete;

        /// Default move constructor.
        segregator(segregator&&) = default;

        /// Deleted copy assignment operator.
        segregator& operator=(segregator const&) = delete;

        /// Default move assignment operator.
        segregator& operator=(segregator&&) = default;

        /**
         * @brief Allocate memory using the small allocator if the size is less than or equal to the `Threshold`, otherwise
         * use the big allocator.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(size_t s) noexcept { return s <= Threshold ? small.allocate(s) : big.allocate(s); }

        /**
         * @brief Allocate memory and set the memory to zero using the small allocator if the size is less than or equal to
         * the `Threshold`, otherwise use the big allocator.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(size_t s) noexcept { return s <= Threshold ? small.allocate_zero(s) : big.allocate_zero(s); }

        /**
         * @brief Deallocate memory using the small allocator if the size is less than or equal to the `Threshold`,
         * otherwise use the big allocator.
         *
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept { return b.s <= Threshold ? small.deallocate(b) : big.deallocate(b); }

        /**
         * @brief Check if a given enda::mem::blk_t memory block is owned by the allocator.
         *
         * @param b enda::mem::blk_t memory block.
         * @return True if one of the two allocators owns the memory block.
         */
        [[nodiscard]] bool owns(blk_t b) const noexcept { return small.owns(b) or big.owns(b); }
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
        long memory_used = 0;

    public:
        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /// Default constructor.
        leak_check() = default;

        /// Deleted copy constructor.
        leak_check(leak_check const&) = delete;

        /// Default move constructor.
        leak_check(leak_check&&) = default;

        /// Deleted copy assignment operator.
        leak_check& operator=(leak_check const&) = delete;

        /// Default move assignment operator.
        leak_check& operator=(leak_check&&) = default;

        /**
         * @brief Destructor that checks for memory leaks.
         * @details In debug mode, it aborts the program if there is a memory leak.
         */
        ~leak_check()
        {
            if (!empty())
            {
#ifndef NDEBUG
                std::cerr << "Memory leak in allocator: " << memory_used << " bytes leaked\n";
                std::abort();
#endif
            }
        }

        /**
         * @brief Allocate memory and update the total memory used.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(size_t s)
        {
            blk_t b = A::allocate(s);
            memory_used += b.s;
            return b;
        }

        /**
         * @brief Allocate memory, set it to zero and update the total memory used.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(size_t s)
        {
            blk_t b = A::allocate_zero(s);
            memory_used += b.s;
            return b;
        }

        /**
         * @brief Deallocate memory and update the total memory used.
         * @details In debug mode, it aborts the program if the total memory used is smaller than zero.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept
        {
            memory_used -= b.s;
            if (memory_used < 0)
            {
#ifndef NDEBUG
                std::cerr << "Memory used by allocator < 0: Memory block to be deleted: b.s = " << b.s << ", b.ptr = " << (void*)b.ptr << "\n";
                std::abort();
#endif
            }
            A::deallocate(b);
        }

        /**
         * @brief Check if the base allocator is empty.
         * @return True if no memory is currently being used.
         */
        [[nodiscard]] bool empty() const { return (memory_used == 0); }

        /**
         * @brief Check if a given enda::mem::blk_t memory block is owned by the base allocator.
         *
         * @param b enda::mem::blk_t memory block.
         * @return True if the base allocator owns the memory block.
         */
        [[nodiscard]] bool owns(blk_t b) const noexcept { return A::owns(b); }

        /**
         * @brief Get the total memory used by the base allocator.
         * @return The size of the memory which has been allocated and not yet deallocated.
         */
        [[nodiscard]] long get_memory_used() const noexcept { return memory_used; }
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
        std::vector<uint64_t> hist = std::vector<uint64_t>(65, 0);

    public:
        /// enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        /// Default constructor.
        stats() = default;

        /// Deleted copy constructor.
        stats(stats const&) = delete;

        /// Default move constructor.
        stats(stats&&) = default;

        /// Deleted copy assignment operator.
        stats& operator=(stats const&) = delete;

        /// Default move assignment operator.
        stats& operator=(stats&&) = default;

        /// Destructor that outputs the statistics about the memory allocation in debug mode.
        ~stats()
        {
#ifndef NDEBUG
            print_histogram(std::cerr);
#endif
        }

        /**
         * @brief Allocate memory and update the histogram.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(uint64_t s)
        {
            // __builtin_clzl returns the number of leading zeros
            ++hist[__builtin_clzl(s)];
            return A::allocate(s);
        }

        /**
         * @brief Allocate memory, set it to zero and update the histogram.
         *
         * @param s Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(uint64_t s)
        {
            // __builtin_clzl returns the number of leading zeros
            ++hist[__builtin_clzl(s)];
            return A::allocate_zero(s);
        }

        /**
         * @brief Deallocate memory.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept { A::deallocate(b); }

        /**
         * @brief Check if a given enda::mem::blk_t memory block is owned by the base allocator.
         *
         * @param b enda::mem::blk_t memory block.
         * @return True if the base allocator owns the memory block.
         */
        [[nodiscard]] bool owns(blk_t b) const noexcept { return A::owns(b); }

        /**
         * @brief Get the histogram of the allocation sizes.
         * @return std::vector of size 65 with the number of allocations in each size range.
         */
        [[nodiscard]] auto const& histogram() const noexcept { return hist; }

        /**
         * @brief Print the histogram to a std::ostream.
         * @param os std::ostream object to print to.
         */
        void print_histogram(std::ostream& os) const
        {
            os << "Allocation size histogram :\n";
            os << "[0, 2^0): " << hist.back() << "\n";
            for (int i = 0; i < 64; ++i)
            {
                os << "[2^" << i << ", 2^" << i + 1 << "): " << hist[63 - i] << "\n";
            }
        }
    };

    /** @} */

} // namespace enda::mem
