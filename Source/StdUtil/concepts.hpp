#pragma once

#include <version>

// libcpp below 13 has incomplete <concepts>
#if defined(_LIBCPP_VERSION) and _LIBCPP_VERSION < 13000

    #include <type_traits>

namespace std
{

    template<class T>
    concept integral = std::is_integral_v<T>;

} // namespace std

#endif
