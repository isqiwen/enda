#pragma once

#include <complex>
#include <concepts>
#include <type_traits>

using namespace std::literals::complex_literals;

namespace std
{

// define operators (+,-,*,/) for std::complex and various other arithmetic types
#define IMPL_OP(OP) \
    /** @brief Implementation of operator `OP` for std::complex and some other arithmetic type. */ \
    template<typename T, typename U> \
    requires(std::is_arithmetic_v<T>and std::is_arithmetic_v<U>and std::common_with<T, U>) auto operator OP(std::complex<T> const& x, U y) \
    { \
        using C = std::complex<std::common_type_t<T, U>>; \
        return C(x.real(), x.imag()) OP C(y); \
    } \
\
    /** @brief Implementation of operator `OP` for some other arithmetic type and std::complex. */ \
    template<typename T, typename U> \
    requires(std::is_arithmetic_v<T>and std::is_arithmetic_v<U>and std::common_with<T, U>) auto operator OP(T x, std::complex<U> const& y) \
    { \
        using C = std::complex<std::common_type_t<T, U>>; \
        return C(x) OP C(y.real(), y.imag()); \
    } \
\
    /** @brief Implementation of operator `OP` for two std::complex types with different value types. */ \
    template<typename T, typename U> \
    requires(std::is_arithmetic_v<T>and std::is_arithmetic_v<U>and std::common_with<T, U> and !std::is_same_v<T, U>) auto operator OP( \
        std::complex<T> const& x, std::complex<U> const& y) \
    { \
        using C = std::complex<std::common_type_t<T, U>>; \
        return C(x.real(), x.imag()) OP C(y.real(), y.imag()); \
    }

    IMPL_OP(+)
    IMPL_OP(-)
    IMPL_OP(*)
    IMPL_OP(/)

#undef IMPL_OP

} // namespace std
