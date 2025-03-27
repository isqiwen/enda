#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(_BitScanReverse)
    #pragma intrinsic(_BitScanReverse64)
#endif

#include "Exceptions.hpp"
#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/ConcurrentBitset.hpp"
#include "Mem/Malloc.hpp"
#include "Mem/Memset.hpp"
#include "Mem/SingletonPool.hpp"

#ifndef NDEBUG
    #include <iostream>
#endif

namespace enda::mem
{
    enum class BlockScale : uint8_t
    {
        e64K = 0,
        e1M,
        e2M,
        e4M,
        e8M,
        e16M,
        e32M,
        e64M,
        e128M,
        e256M,
        e512M,
        eInvalid
    };

    using enum BlockScale;

    // Memory block consisting of a pointer and its size.
    struct blk_t
    {
        // Pointer to the memory block.
        char* ptr = nullptr;

        // Size of the memory block in bytes.
        std::size_t s = 0;

        // memory pool scale(optional)
        BlockScale scale = eInvalid;
    };

    /**
     * @brief Custom allocator that uses enda::mem::malloc to allocate memory.
     * @tparam AdrSp enda::mem::AddressSpace in which the memory is allocated.
     */
    template<AddressSpace AdrSp = Host>
    class mallocator : public enda::singleton<mallocator<AdrSp>>
    {
    public:
        // enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = AdrSp;

        static void init() { return; }

        static void release() { return; }

        // alloc_size: Size in bytes of the memory to allocate.
        static blk_t allocate(std::size_t alloc_size) noexcept { return blk_t {(char*)malloc<address_space>(alloc_size), alloc_size}; }

        static blk_t allocate_zero(std::size_t alloc_size) noexcept
        {
            blk_t b = allocate(alloc_size);
            memset<address_space>((void*)b.ptr, 0, alloc_size);
            return b;
        }

        static void deallocate(blk_t b) noexcept { free<address_space>((void*)b.ptr); }
    };

    constexpr std::size_t _64K_LG2  = 16;
    constexpr std::size_t _1M_LG2   = 20;
    constexpr std::size_t _2M_LG2   = 21;
    constexpr std::size_t _4M_LG2   = 22;
    constexpr std::size_t _8M_LG2   = 23;
    constexpr std::size_t _16M_LG2  = 24;
    constexpr std::size_t _32M_LG2  = 25;
    constexpr std::size_t _64M_LG2  = 26;
    constexpr std::size_t _128M_LG2 = 27;
    constexpr std::size_t _256M_LG2 = 28;
    constexpr std::size_t _512M_LG2 = 29;

    constexpr std::size_t _64K  = 1 << _64K_LG2;
    constexpr std::size_t _1M   = 1 << _1M_LG2;
    constexpr std::size_t _2M   = 1 << _2M_LG2;
    constexpr std::size_t _4M   = 1 << _4M_LG2;
    constexpr std::size_t _8M   = 1 << _8M_LG2;
    constexpr std::size_t _16M  = 1 << _16M_LG2;
    constexpr std::size_t _32M  = 1 << _32M_LG2;
    constexpr std::size_t _64M  = 1 << _64M_LG2;
    constexpr std::size_t _128M = 1 << _128M_LG2;
    constexpr std::size_t _256M = 1 << _256M_LG2;
    constexpr std::size_t _512M = 1 << _512M_LG2;

    template<AddressSpace AdrSp = Host>
    class multi_scale_singleton_pool : public enda::singleton<multi_scale_singleton_pool<AdrSp>>
    {
    public:
        static constexpr auto address_space = AdrSp;

        ENDA_CREATE_POOL(e64K, _64K_LG2, 11);  // 64K * 2048 = 128M
        ENDA_CREATE_POOL(e1M, _1M_LG2, 10);    // 1M  * 1024 = 1G
        ENDA_CREATE_POOL(e2M, _2M_LG2, 9);     // 2M  *  512 = 1G
        ENDA_CREATE_POOL(e4M, _4M_LG2, 8);     // 4M  *  256 = 1G
        ENDA_CREATE_POOL(e8M, _8M_LG2, 7);     // 8M  *  128 = 1G
        ENDA_CREATE_POOL(e16M, _16M_LG2, 6);   // 16M  *  64 = 1G
        ENDA_CREATE_POOL(e32M, _32M_LG2, 5);   // 32M  *  32 = 1G
        ENDA_CREATE_POOL(e64M, _64M_LG2, 5);   // 64M  *  32 = 2G
        ENDA_CREATE_POOL(e128M, _128M_LG2, 4); // 128M *  16 = 2G
        ENDA_CREATE_POOL(e256M, _256M_LG2, 5); // 256M *  32 = 8G
        ENDA_CREATE_POOL(e512M, _512M_LG2, 4); // 512M *  16 = 8G

        static void init()
        {
            ENDA_INIT_POOL(e64K);
            ENDA_INIT_POOL(e1M);
            ENDA_INIT_POOL(e2M);
            ENDA_INIT_POOL(e4M);
            ENDA_INIT_POOL(e8M);
            ENDA_INIT_POOL(e16M);
            ENDA_INIT_POOL(e32M);
            ENDA_INIT_POOL(e64M);
            ENDA_INIT_POOL(e128M);
            ENDA_INIT_POOL(e256M);
            ENDA_INIT_POOL(e512M);
        }

        static void release()
        {
            ENDA_RELEASE_POOL(e64K);
            ENDA_RELEASE_POOL(e1M);
            ENDA_RELEASE_POOL(e2M);
            ENDA_RELEASE_POOL(e4M);
            ENDA_RELEASE_POOL(e8M);
            ENDA_RELEASE_POOL(e16M);
            ENDA_RELEASE_POOL(e32M);
            ENDA_RELEASE_POOL(e64M);
            ENDA_RELEASE_POOL(e128M);
            ENDA_RELEASE_POOL(e256M);
            ENDA_RELEASE_POOL(e512M);
        }

        static blk_t allocate(std::size_t alloc_size) noexcept
        {
            BlockScale raw_scale = calculate_scale(alloc_size);

            if (raw_scale == eInvalid)
            {
                return blk_t {nullptr, 0, eInvalid};
            }

            char* p = nullptr;

            for (int i = 0; i < s_attempt_limit; ++i)
            {
                BlockScale scale = raw_scale;

                while (scale < eInvalid)
                {
                    switch (scale)
                    {
                        case BlockScale::e64K: {
                            p = ENDA_GET_POOL(e64K).allocate();
                            break;
                        }
                        case BlockScale::e1M: {
                            p = ENDA_GET_POOL(e1M).allocate();
                            break;
                        }
                        case BlockScale::e2M: {
                            p = ENDA_GET_POOL(e2M).allocate();
                            break;
                        }
                        case BlockScale::e4M: {
                            p = ENDA_GET_POOL(e4M).allocate();
                            break;
                        }
                        case BlockScale::e8M: {
                            p = ENDA_GET_POOL(e8M).allocate();
                            break;
                        }
                        case BlockScale::e16M: {
                            p = ENDA_GET_POOL(e16M).allocate();
                            break;
                        }
                        case BlockScale::e32M: {
                            p = ENDA_GET_POOL(e32M).allocate();
                            break;
                        }
                        case BlockScale::e64M: {
                            p = ENDA_GET_POOL(e64M).allocate();
                            break;
                        }
                        case BlockScale::e128M: {
                            p = ENDA_GET_POOL(e128M).allocate();
                            break;
                        }
                        case BlockScale::e256M: {
                            p = ENDA_GET_POOL(e256M).allocate();
                            break;
                        }
                        case BlockScale::e512M: {
                            p = ENDA_GET_POOL(e512M).allocate();
                            break;
                        }
                        default:
                            abort("Invalid scale");
                    }

                    if (nullptr != p)
                    {
                        return blk_t {p, alloc_size, scale};
                    }

                    scale = static_cast<BlockScale>(static_cast<uint8_t>(scale) + 1);
                }
            }

            return blk_t {nullptr, 0, eInvalid};
        }

        static blk_t allocate_zero(std::size_t alloc_size) noexcept
        {
            blk_t b = allocate(alloc_size);
            memset<address_space>((void*)b.ptr, 0, alloc_size);

            return b;
        }

        static void deallocate(const blk_t& b) noexcept
        {
            switch (b.scale)
            {
                case BlockScale::e64K: {
                    ENDA_GET_POOL(e64K).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e1M: {
                    ENDA_GET_POOL(e1M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e2M: {
                    ENDA_GET_POOL(e2M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e4M: {
                    ENDA_GET_POOL(e4M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e8M: {
                    ENDA_GET_POOL(e8M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e16M: {
                    ENDA_GET_POOL(e16M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e32M: {
                    ENDA_GET_POOL(e32M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e64M: {
                    ENDA_GET_POOL(e64M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e128M: {
                    ENDA_GET_POOL(e128M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e256M: {
                    ENDA_GET_POOL(e256M).deallocate(b.ptr);
                    break;
                }
                case BlockScale::e512M: {
                    ENDA_GET_POOL(e512M).deallocate(b.ptr);
                    break;
                }
                default:
                    abort("Invalid scale");
            }
        }

    private:
        static BlockScale calculate_scale(size_t alloc_size)
        {
            if (alloc_size <= _64K)
            {
                return e64K;
            }

            if (alloc_size <= _1M)
            {
                return e1M;
            }

            if (alloc_size <= _2M)
            {
                return e2M;
            }

            if (alloc_size <= _4M)
            {
                return e4M;
            }

            if (alloc_size <= _8M)
            {
                return e8M;
            }

            if (alloc_size <= _16M)
            {
                return e16M;
            }

            if (alloc_size <= _32M)
            {
                return e32M;
            }

            if (alloc_size <= _64M)
            {
                return e64M;
            }

            if (alloc_size <= _128M)
            {
                return e128M;
            }

            if (alloc_size <= _256M)
            {
                return e256M;
            }

            if (alloc_size <= _512M)
            {
                return e512M;
            }

            return eInvalid;
        }

    private:
        static constexpr uint32_t s_attempt_limit = 10;
    };

    struct AllocationRecord
    {
        std::size_t size;
        std::string file;
        int         line;
    };

    /**
     * @brief Wrap an allocator to check for memory usage statistics.
     *
     * @details It simply keeps track of the memory currently being used by the allocator, i.e. the total memory allocated
     * minus the total memory deallocated, which should never be smaller than zero and should be exactly zero when the
     * allocator is destroyed.
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
    public:
        // enda::mem::AddressSpace in which the memory is allocated.
        static constexpr auto address_space = A::address_space;

        using A::A;

        ~stats()
        {
            std::scoped_lock lock(m_mutex);
            if (!m_allocations.empty())
            {
                std::cerr << "Memory leak detected. Leaked allocations:\n";
                for (const auto& [ptr, record] : m_allocations)
                {
                    std::cerr << "  Leaked pointer: " << ptr << ", size: " << record.size << ", allocated at: " << record.file << ":" << record.line << "\n";
                }
            }

            print_histogram(std::cout);
        }

        static void init() { A::init(); }

        static void release() { A::release(); }

        /**
         * @brief Allocate memory and update the total memory used.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate(std::size_t alloc_size, const char* file, int line) noexcept
        {
            blk_t b = A::allocate(alloc_size);

            if (b.ptr)
            {
                std::scoped_lock lock(m_mutex);
                m_allocations.emplace(b.ptr, AllocationRecord {b.s, file, line});

                if (alloc_size == 0)
                {
                    m_hist.back() += 1;
                }
                else
                {
                    m_hist[std::countl_zero(alloc_size)] += 1;
                }
            }

            return b;
        }

        /**
         * @brief Allocate memory, set it to zero and update the total memory used.
         *
         * @param alloc_size Size in bytes of the memory to allocate.
         * @return enda::mem::blk_t memory block.
         */
        blk_t allocate_zero(std::size_t alloc_size, const char* file, int line) noexcept
        {
            blk_t b = A::allocate_zero(alloc_size);

            if (nullptr != b.ptr)
            {
                std::scoped_lock lock(m_mutex);
                m_allocations.emplace(b.ptr, AllocationRecord {b.s, file, line});

                if (alloc_size == 0)
                {
                    m_hist.back() += 1;
                }
                else
                {
                    m_hist[std::countl_zero(alloc_size)] += 1;
                }
            }

            return b;
        }

        /**
         * @brief Deallocate memory and update the total memory used.
         * @details In debug mode, it aborts the program if the total memory used is smaller than zero.
         * @param b enda::mem::blk_t memory block to deallocate.
         */
        void deallocate(blk_t b) noexcept
        {
            if (nullptr == b.ptr)
            {
                return;
            }

            {
                std::scoped_lock lock(m_mutex);
                auto             it = m_allocations.find(b.ptr);
                if (it != m_allocations.end())
                {
                    m_allocations.erase(it);
                }
                else
                {
                    std::cerr << "Warning: deallocating pointer not found in allocation records.\n";
                }
            }

            A::deallocate(b);
        }

        /**
         * @brief Check if the base allocator is empty.
         * @return True if no memory is currently being used.
         */
        [[nodiscard]] bool empty() const
        {
            std::scoped_lock lock(m_mutex);
            return m_allocations.empty();
        }

        /**
         * @brief Get the total memory used by the base allocator.
         * @return The size of the memory which has been allocated and not yet deallocated.
         */
        [[nodiscard]] long get_memory_used() const noexcept
        {
            std::scoped_lock lock(m_mutex);

            std::size_t used = 0;

            for (auto& v : m_allocations)
            {
                used += v.second.size;
            }

            return used;
        }

        void print_histogram(std::ostream& os) const
        {
            os << "Allocation size histogram :\n";
            os << "[0, 2^0): " << m_hist.back() << "\n";
            for (int i = 0; i < 64; ++i)
            {
                os << "[2^" << i << ", 2^" << (i + 1) << "): " << m_hist[63 - i] << "\n";
            }
        }

    private:
        std::unordered_map<void*, AllocationRecord> m_allocations;
        std::array<std::size_t, 65>                 m_hist; // Histogram of the allocation sizes.
        std::mutex                                  m_mutex;
    };

#define POOL_ALLOC(pool, size) (pool).allocate(size)
#define POOL_ALLOC_STATS(pool, size) (pool).allocate((size), __FILE__, __LINE__)

} // namespace enda::mem
