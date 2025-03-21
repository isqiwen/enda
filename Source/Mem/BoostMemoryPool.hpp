#pragma once

#include <boost/pool/singleton_pool.hpp>

#define GET_POOL(scale) MemoryPool##scale

#define CREATE_POOL(size, scale, cnt) \
    class DataObjMemoryPool##scale \
    {}; \
    using MemoryPool##scale = \
        boost::singleton_pool<DataObjMemoryPool##scale, size, boost::default_user_allocator_new_delete, boost::details::pool::default_mutex, cnt>;

#define INIT_POOL(size, scale) \
    { \
        auto buf = GET_POOL(scale)::ordered_malloc(); \
        if (buf) \
        { \
            GET_POOL(scale)::ordered_free(buf); \
        } \
    }

#define RELEASE_POOL(scale) \
    { \
        GET_POOL(scale)::release_memory(); \
    }

#define MALLOC(buf, size, scale) \
    { \
        auto blockSize = (size_t)std::ceil((double)size / (double)GET_POOL(scale)::requested_size); \
        buf            = GET_POOL(scale)::ordered_malloc(blockSize); \
    }

#define FREE(ptr, size, scale) \
    { \
        auto blockSize = (size_t)std::ceil((double)size / (double)GET_POOL(scale)::requested_size); \
        if (GET_POOL(scale)::is_from(ptr)) \
        { \
            GET_POOL(scale)::ordered_free(ptr, blockSize); \
        } \
    }

enum eBlockScale
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
    e512M
};

const int64_t _64K  = 64 * 1024;
const int64_t _1M   = 1024 * 1024;
const int64_t _2M   = 2 * _1M;
const int64_t _4M   = 2 * _2M;
const int64_t _8M   = 2 * _4M;
const int64_t _16M  = 2 * _8M;
const int64_t _32M  = 2 * _16M;
const int64_t _64M  = 2 * _32M;
const int64_t _128M = 2 * _64M;
const int64_t _256M = 2 * _128M;
const int64_t _512M = 2 * _256M;

class MemoryPool
{
public:
    CREATE_POOL(_64K, 64K, 1024 * 2); // 128M
    CREATE_POOL(_1M, 1M, 1024);       // 1G
    CREATE_POOL(_2M, 2M, 512);        // 1G
    CREATE_POOL(_4M, 4M, 256);        // 1G
    CREATE_POOL(_8M, 8M, 128);        // 1G
    CREATE_POOL(_16M, 16M, 64);       // 1G
    CREATE_POOL(_32M, 32M, 32);       // 1G
    CREATE_POOL(_64M, 64M, 32);       // 2G
    CREATE_POOL(_128M, 128M, 16);     // 2G
    CREATE_POOL(_256M, 256M, 32);     // 8G
    CREATE_POOL(_512M, 512M, 16);     // 8G

    static void init()
    {
        INIT_POOL(_64K, 64K);   // 128M
        INIT_POOL(_1M, 1M);     // 1G
        INIT_POOL(_2M, 2M);     // 1G
        INIT_POOL(_4M, 4M);     // 1G
        INIT_POOL(_8M, 8M);     // 1G
        INIT_POOL(_16M, 16M);   // 1G
        INIT_POOL(_32M, 32M);   // 1G
        INIT_POOL(_64M, 64M);   // 2G
        INIT_POOL(_128M, 128M); // 2G
        INIT_POOL(_256M, 256M); // 8G
        INIT_POOL(_512M, 512M); // 8G
    }

    static void release()
    {
        RELEASE_POOL(64K);
        RELEASE_POOL(1M);
        RELEASE_POOL(2M);
        RELEASE_POOL(4M);
        RELEASE_POOL(8M);
        RELEASE_POOL(16M);
        RELEASE_POOL(32M);
        RELEASE_POOL(64M);
        RELEASE_POOL(128M);
        RELEASE_POOL(256M);
        RELEASE_POOL(512M);
    }

    static void* Malloc(const size_t size, eBlockScale& scale)
    {
        scale    = CalculateScale(size);
        auto buf = RecursiveMalloc(size, scale);
        return buf;
    }

    static eBlockScale CalculateScale(const size_t size)
    {
        if (size < _1M)
        {
            return eBlockScale::e64K;
        }
        auto count  = std::ceil((double)size / (double)_1M);
        auto iScale = (int)std::ceil(std::log2(count)) + 1;
        iScale      = (iScale > (int)e512M) ? int(e512M) : iScale;
        return eBlockScale(iScale);
    }

    static void* RecursiveMalloc(const size_t size, eBlockScale& scale)
    {
        void* spBuf = nullptr;
        switch (scale)
        {
            case eBlockScale::e64K:
                MALLOC(spBuf, size, 64K);
                break;
            case eBlockScale::e1M:
                MALLOC(spBuf, size, 1M);
                break;
            case eBlockScale::e2M:
                MALLOC(spBuf, size, 2M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e4M:
                MALLOC(spBuf, size, 4M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e8M:
                MALLOC(spBuf, size, 8M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e16M:
                MALLOC(spBuf, size, 16M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e32M:
                MALLOC(spBuf, size, 32M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e64M:
                MALLOC(spBuf, size, 64M);
                break;
            case eBlockScale::e128M:
                MALLOC(spBuf, size, 128M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e256M:
                MALLOC(spBuf, size, 256M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            case eBlockScale::e512M:
                MALLOC(spBuf, size, 512M);
                if (!spBuf)
                {
                    scale = eBlockScale(scale - 1);
                    spBuf = RecursiveMalloc(size, scale);
                }
                break;
            default:
                break;
        }
        return spBuf;
    }

    static void Free(void* ptr, size_t size, eBlockScale scale)
    {
        switch (scale)
        {
            case eBlockScale::e64K:
                FREE(ptr, size, 64K);
                break;
            case eBlockScale::e1M:
                FREE(ptr, size, 1M);
                break;
            case eBlockScale::e2M:
                FREE(ptr, size, 2M);
                break;
            case eBlockScale::e4M:
                FREE(ptr, size, 4M);
                break;
            case eBlockScale::e8M:
                FREE(ptr, size, 8M);
                break;
            case eBlockScale::e16M:
                FREE(ptr, size, 16M);
                break;
            case eBlockScale::e32M:
                FREE(ptr, size, 32M);
                break;
            case eBlockScale::e64M:
                FREE(ptr, size, 64M);
                break;
            case eBlockScale::e128M:
                FREE(ptr, size, 128M);
                break;
            case eBlockScale::e256M:
                FREE(ptr, size, 256M);
                break;
            case eBlockScale::e512M:
                FREE(ptr, size, 512M);
                break;
            default:
                break;
        }
    }
};
