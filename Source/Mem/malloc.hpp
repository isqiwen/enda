#pragma once

#include <cstdlib>

#include "Device.hpp"
#include "Mem/AddressSpace.hpp"

namespace enda::mem
{
    /**
     * @brief Call the correct `malloc` function based on the given address space.
     *
     * @details It makes the following function calls depending on the address space:
     * - `std::malloc` for `Host`.
     * - `cudaMalloc` for `Device`.
     * - `cudaMallocManaged` for `Unified`.
     *
     * @tparam AdrSp enda::mem::AddressSpace.
     * @param size Size in bytes to be allocated.
     * @return Pointer to the allocated memory.
     */
    template<AddressSpace AdrSp>
    void* malloc(size_t size)
    {
        check_adr_sp_valid<AdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        void* ptr = nullptr;
        if constexpr (AdrSp == Host)
        {
            ptr = std::malloc(size);
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
            std::free(p);
        }
        else
        {
            device_error_check(cudaFree(p), "cudaFree");
        }
    }

} // namespace enda::mem
