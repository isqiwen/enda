#pragma once

#include <bit>
#include <cstdint>
#include <string>

namespace enda
{

// ---------------- Constants ---------------- //

/**
 * @brief The cache line size (bytes) of the current architecture.
 */
#ifdef __cpp_lib_hardware_interference_size
    inline constexpr std::size_t k_cache_line = std::hardware_destructive_interference_size;
#else
    inline constexpr std::size_t k_cache_line = 64;
#endif

    template<typename T>
    std::string to_binary(T num) noexcept
    {
        static_assert(std::is_integral_v<T>, "to_binary requires an integral type");

        if (num == 0)
        {
            return "0";
        }

        std::string binary;
        using UnsignedT   = std::make_unsigned_t<T>;
        auto unsigned_num = std::bit_cast<UnsignedT>(num);

        while (unsigned_num != 0)
        {
            binary = (unsigned_num & 1 ? "1" : "0") + binary;
            unsigned_num >>= 1;
        }

        if constexpr (std::is_signed_v<T>)
        {
            if (num < 0)
            {
                size_t bit_width = sizeof(T) * 8;
                binary           = std::string(bit_width - binary.length(), '1') + binary;
            }
        }

        return binary;
    }

} // namespace enda
