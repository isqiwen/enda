#pragma once

#include <cstdlib>
#include <iostream>

namespace enda
{
    /**
     * @brief Trigger a compilation error in case GPU specific functionality is used without configuring the project with
     * GPU support.
     */
    template<bool flag = false>
    void compile_error_no_gpu()
    {
        static_assert(flag, "Using device functionality without gpu support! Configure project with -DCudaSupport=ON.");
    }

#ifdef ENDA_HAVE_CUDA
    // TODO:

    // Constexpr variable that is true if the project is configured with GPU support.
    inline constexpr bool have_device = true;

    // Constexpr variable that is true if the project is configured with CUDA support.
    inline constexpr bool have_cuda = true;
#else

    // Trigger a compilation error every time the enda::device_error_check function is called.
    #define device_error_check(ARG1, ARG2) compile_error_no_gpu()

    // Constexpr variable that is true if the project is configured with GPU support.
    inline constexpr bool have_device = false;

    // Constexpr variable that is true if the project is configured with CUDA support.
    inline constexpr bool have_cuda = false;

#endif

    void host_abort(const char* const message)
    {
        std::cerr << message << std::endl;
        ::abort();
    }

    [[noreturn]] inline void abort(const char* const message) { host_abort(message); }

    [[noreturn]] inline void abort(const std::string& message) { host_abort(message.c_str()); }

} // namespace enda
