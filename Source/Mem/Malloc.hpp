#pragma once

#include "Device.hpp"
#include "Mem/AddressSpace.hpp"
#include "Utility.hpp"

namespace enda::mem
{

#ifdef _MSC_VER
    #include <malloc.h>
    inline void* aligned_alloc(std::size_t alignment, std::size_t size) { return _aligned_malloc(size, alignment); }
    inline void  aligned_free(void* ptr) { _aligned_free(ptr); }
#else
    #include <stdlib.h>
    inline void aligned_free(void* ptr) { free(ptr); }
#endif

    /**
     * @brief Call the correct `malloc` function based on the given address space.
     *
     * @details It makes the following function calls depending on the address space:
     * - `aligned_alloc` for `Host`.
     * - `cudaMalloc` for `Device`.
     * - `cudaMallocManaged` for `Unified`.
     *
     * @tparam AdrSp enda::mem::AddressSpace.
     * @param size Size in bytes to be allocated.
     * @return Pointer to the allocated memory.
     */
    template<AddressSpace AdrSp>
    void* malloc(std::size_t size)
    {
        check_adr_sp_valid<AdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        if (0 == size)
        {
            return nullptr;
        }

        void* ptr = nullptr;
        if constexpr (AdrSp == Host)
        {
            ptr = aligned_alloc(k_cache_line, size);
        }
        else if constexpr (AdrSp == Device)
        {
            device_error_check(cudaMalloc((void**)&ptr, size), "cudaMalloc");
        }
        else
        {
            device_error_check(cudaMallocManaged((void**)&ptr, size), "cudaMallocManaged");
        }
        return ptr;
    }

    /**
     * @brief Call the correct `free` function based on the given address space.
     *
     * @details It makes the following function calls depending on the address space:
     * - `std::free` for `Host`.
     * - `cudaFree` for `Device` and `Unified`.
     *
     * @tparam AdrSp enda::mem::AddressSpace.
     * @param p Pointer to the memory to be freed.
     */
    template<AddressSpace AdrSp>
    void free(void* p)
    {
        check_adr_sp_valid<AdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        if constexpr (AdrSp == Host)
        {
            aligned_free(p);
        }
        else
        {
            device_error_check(cudaFree(p), "cudaFree");
        }
    }

    template<AddressSpace AdrSp>
    struct PtrDeleter
    {
        void operator()(void* ptr) const
        {
            if (ptr)
            {
                free<AdrSp>(ptr);
            }
        }
    };

} // namespace enda::mem
