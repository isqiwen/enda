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
    template<AddressSpace AdrSp, std::size_t Alignment = 0>
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
            if constexpr (Alignment == 0)
            {
                ptr = std::malloc(size);
            }
            else
            {
                ptr = aligned_alloc(Alignment, size);
            }
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
    template<AddressSpace AdrSp, std::size_t Alignment = 0>
    void free(void* p)
    {
        check_adr_sp_valid<AdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        if constexpr (AdrSp == Host)
        {
            if constexpr (Alignment == 0)
            {
                std::free(p);
            }
            else
            {
                aligned_free(p);
            }
        }
        else
        {
            device_error_check(cudaFree(p), "cudaFree");
        }
    }

    template<AddressSpace AdrSp, std::size_t Alignment = 0>
    struct ptr_deleter
    {
        void operator()(void* ptr) const
        {
            if (ptr)
            {
                free<AdrSp, Alignment>(ptr);
            }
        }
    };

    bool is_aligned(void* ptr, std::size_t alignment) {return reinterpret_cast<std::uintptr_t>(ptr) % alignment == 0; }

} // namespace enda::mem
