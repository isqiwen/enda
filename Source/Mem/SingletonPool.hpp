#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

#include "Exceptions.hpp"
#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/ConcurrentBitset.hpp"
#include "Mem/Malloc.hpp"
#include "Mem/Memset.hpp"
#include "Singleton.h"

namespace enda::mem
{
    template<typename Tag, std::size_t BlockSizeL2, std::size_t BlockCntL2, AddressSpace AdrSp = Host>
    class singleton_pool : public enda::singleton<singleton_pool<Tag, BlockSizeL2, BlockCntL2, AdrSp>>
    {
    public:
        using size_type = std::size_t;

        bool init() noexcept
        {
            void* raw_buffer = malloc<s_address_space, s_alignment>(s_total_size);

            if (nullptr == raw_buffer)
            {
                return false;
            }

            memset<s_address_space>(raw_buffer, 0, s_bitset_words * sizeof(uint32_t));

            m_buffer.reset((char*)raw_buffer);

            *status_buffer() = BlockCntL2 << concurrent_bitset::state_shift;

            return true;
        }

        char* allocate()
        {
            auto res = concurrent_bitset::acquire_bounded_lg2(status_buffer(), static_cast<uint32_t>(BlockCntL2));
            int  bit = res.first;

            if (bit < 0)
            {
                return nullptr;
            }

            return data_buffer() + (static_cast<size_type>(bit) << BlockSizeL2);
        }

        void deallocate(char* ptr) noexcept
        {
            if (nullptr == ptr)
            {
                return;
            }

            ptrdiff_t offset = ptr - data_buffer();

            if (offset < 0 || offset > s_data_size - s_block_size)
            {
                abort("Deallocation error: pointer offset out of bounds of the memory pool data region.");
            }

            if ((offset & (s_block_size - 1)) != 0)
            {
                abort("Deallocation error: pointer is not aligned to the start of a block.");
            }

            const uint32_t bit    = offset >> BlockSizeL2;
            const int      result = concurrent_bitset::release(status_buffer(), bit);

            bool ok_dealloc_once = 0 <= result;

            if (!ok_dealloc_once)
            {
                abort("Deallocation error: block at given pointer was already freed or was not allocated from this pool.");
            }
        }

        void release_memory() noexcept { m_buffer.reset(); }

        void purge_memory() noexcept
        {
            memset<s_address_space>(reinterpret_cast<void*>(status_buffer()), 0, s_bitset_words * sizeof(uint32_t));
            *status_buffer() = BlockCntL2 << concurrent_bitset::state_shift;
        }

    private:
        inline uint32_t* status_buffer() noexcept { return reinterpret_cast<uint32_t*>(m_buffer.get()); }

        inline char* data_buffer() noexcept { return m_buffer.get() + sizeof(uint32_t) * s_bitset_words; }

    private:
        static constexpr auto      s_address_space     = AdrSp;
        static constexpr size_type s_alignment         = k_cache_line;
        static constexpr size_type s_uint32_align_mask = s_alignment / sizeof(uint32_t) - 1;
        static constexpr size_type s_block_size        = 1ULL << BlockSizeL2;
        static constexpr size_type s_bitset_words      = (concurrent_bitset::buffer_bound_lg2(BlockCntL2) + s_uint32_align_mask) & ~s_uint32_align_mask;
        static constexpr size_type s_data_size         = s_block_size << BlockCntL2;
        static constexpr size_type s_total_size        = sizeof(uint32_t) * s_bitset_words + s_data_size;

        std::unique_ptr<char, ptr_deleter<s_address_space, s_alignment>> m_buffer = nullptr;
    };

} // namespace enda::mem

#define ENDA_CREATE_POOL(tag, block_size, block_count) \
    struct tag_singleton_pool_##tag \
    {}; \
    using singleton_pool_##tag = enda::mem::singleton_pool<tag_singleton_pool_##tag, block_size, block_count>;

#define ENDA_GET_POOL(tag) singleton_pool_##tag::instance()

#define ENDA_INIT_POOL(tag) \
    { \
        if (!ENDA_GET_POOL(tag).init()) \
        { \
            enda::abort(std::string("singleton_pool_") + #tag + " init failed."); \
        } \
    }

#define ENDA_RELEASE_POOL(tag) \
    { \
        ENDA_GET_POOL(tag).release_memory(); \
    }
