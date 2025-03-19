#pragma once

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

} // namespace enda
