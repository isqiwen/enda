#pragma once

namespace enda
{

#ifdef ENDA_HAVE_CUDA
    // TODO:
#else
    /// Constexpr variable that is true if the project is configured with GPU support.
    inline constexpr bool have_device = false;

    /// Constexpr variable that is true if the project is configured with CUDA support.
    inline constexpr bool have_cuda = false;

#endif

} // namespace enda
