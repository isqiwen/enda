#pragma once

#include <version>

// libcpp below 13 has incomplete <concepts>
#if defined(_LIBCPP_VERSION) and _LIBCPP_VERSION < 13000

    #include <type_traits>

namespace std
{

    template<class T>
    concept integral = std::is_integral_v<T>;

    static_assert(std::integral<int>);
    static_assert(std::integral<unsigned long>);
    static_assert(std::integral<int64_t>);

    static_assert(!std::integral<double>);
    static_assert(!std::integral<void>);
    static_assert(!std::integral<int*>);

} // namespace std

#endif
