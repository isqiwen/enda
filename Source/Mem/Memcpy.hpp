#pragma once

#include <cstring>

#include "Device.hpp"
#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"

namespace enda::mem
{
    /**
     * @brief Call the correct `memcpy` function based on the given address spaces.
     *
     * @details It makes the following function calls depending on the address spaces:
     * - `std::memcpy` if both address spaces are `Host`.
     * - `cudaMemcpy` for all other combinations.
     *
     * @tparam DestAdrSp enda::mem::AddressSpace of the destination.
     * @tparam SrcAdrSp enda::mem::AddressSpace of the source.
     * @param dest Pointer to the destination memory.
     * @param src Pointer to the source memory.
     * @param count Size in bytes to copy.
     */
    template<AddressSpace DestAdrSp, AddressSpace SrcAdrSp>
    void memcpy(void* dest, void const* src, size_t count)
    {
        check_adr_sp_valid<DestAdrSp, SrcAdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        if constexpr (DestAdrSp == Host && SrcAdrSp == Host)
        {
            std::memcpy(dest, src, count);
        }
        else
        {
            device_error_check(cudaMemcpy(dest, src, count, cudaMemcpyDefault), "cudaMemcpy");
        }
    }

    /**
     * @brief Call CUDA's `cudaMemcpy2D` function or simulate its behavior on the `Host` based on the given address
     * spaces.
     *
     * @details Copies a matrix (`height` rows of `width` bytes each) from the memory area pointed to by `src` to the
     * memory area pointed to by `dest`. `dpitch` and `spitch` are the widths in memory in bytes of the 2D arrays pointed
     * to by `dest` and `src`, including any padding added to the end of each row. The memory areas may not overlap.
     * `width` must not exceed either `dpitch` or `spitch`.
     *
     * If both address spaces are `Host`, it simulates the behavior of CUDA's `cudaMemcpy2D` function by making multiple
     * calls to std::memcpy.
     *
     * @tparam DestAdrSp enda::mem::AddressSpace of the destination.
     * @tparam SrcAdrSp enda::mem::AddressSpace of the source.
     * @param dest Pointer to the destination memory.
     * @param dpitch Pitch of destination memory
     * @param src Pointer to the source memory.
     * @param spitch Pitch of source memory.
     * @param width Width of matrix transfer (columns in bytes).
     * @param height Height of matrix transfer (rows).
     */
    template<AddressSpace DestAdrSp, AddressSpace SrcAdrSp>
    void memcpy2D(void* dest, size_t dpitch, const void* src, size_t spitch, size_t width, size_t height)
    {
        EXPECTS(width <= dpitch && width <= spitch);
        check_adr_sp_valid<DestAdrSp, SrcAdrSp>();
        static_assert(enda::have_device == enda::have_cuda, "Adjust function for new device types");

        if constexpr (DestAdrSp == Host && SrcAdrSp == Host)
        {
            auto* desti = static_cast<unsigned char*>(dest);
            auto* srci  = static_cast<const unsigned char*>(src);
            for (size_t i = 0; i < height; ++i, desti += dpitch, srci += spitch)
                std::memcpy(desti, srci, width);
        }
        else if (enda::have_device)
        {
            device_error_check(cudaMemcpy2D(dest, dpitch, src, spitch, width, height, cudaMemcpyDefault), "cudaMemcpy2D");
        }
    }

} // namespace enda::mem
