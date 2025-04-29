#pragma once

/**
 * @file Macros.hpp
 *
 * @brief A collection of internal/public macros.
 *
 * These are exhaustively documented due to macros nasty visibility rules however, only
 * macros that are marked as __[public]__ should be consumed.
 */

#include <exception>
#include <iostream>

/**
 * @brief __[public]__ The major version of enda.
 *
 * Increments with incompatible API changes.
 */
#define ENDA_VERSION_MAJOR 1
/**
 * @brief __[public]__ The minor version of enda.
 *
 * Increments when functionality is added in an API backward compatible manner.
 */
#define ENDA_VERSION_MINOR 0
/**
 * @brief __[public]__ The patch version of enda.
 *
 * Increments when bug fixes are made in an API backward compatible manner.
 */
#define ENDA_VERSION_PATCH 0

/**
 * @brief Detect the operating system
 */
#if defined(_WIN32)
    #define ENDA_WINDOWS // Windows
    #define ENDA_OS_NAME "windows"
#elif defined(_WIN64)
    #define ENDA_WINDOWS // Windows
    #define ENDA_OS_NAME "windows"
#elif defined(__CYGWIN__) && !defined(_WIN32)
    #define ENDA_WINDOWS // Windows (Cygwin POSIX under Microsoft Window)
    #define ENDA_OS_NAME "windows"
#elif defined(__linux__)
    #define ENDA_LINUX // Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
    #define ENDA_UNIX
    #define ENDA_OS_NAME "linux"
#else
    #define ENDA_UNKNOWN
    #define ENDA_OS_NAME "unknown"
#endif

/**
 * @brief Use to conditionally decorate lambdas and ``operator()`` (alongside ``ENDA_STATIC_CONST``) with
 * ``static``.
 */
#ifdef __cpp_static_call_operator
    #define ENDA_STATIC_CALL static
#else
    #define ENDA_STATIC_CALL
#endif

/**
 * @brief Use with ``ENDA_STATIC_CALL`` to conditionally decorate ``operator()`` with ``const``.
 */
#ifdef __cpp_static_call_operator
    #define ENDA_STATIC_CONST
#else
    #define ENDA_STATIC_CONST const
#endif

// ---------------- Stringify ----------------

#define AS_STRING(...) AS_STRING2(__VA_ARGS__)
#define AS_STRING2(...) #__VA_ARGS__

// ---------------- Print ----------------

#define PRINT(X) std::cerr << AS_STRING(X) << " = " << X << "      at " << __FILE__ << ":" << __LINE__ << '\n'
#define ENDA_PRINT(X) std::cerr << AS_STRING(X) << " = " << X << "      at " << __FILE__ << ":" << __LINE__ << '\n'

// ---------------- Inline ----------------

#if !defined(FORCEINLINE)
    #if defined(_MSC_VER) && !defined(__clang__)
        #define FORCEINLINE __forceinline
    #elif defined(__GNUC__) && __GNUC__ > 3
        #define FORCEINLINE __attribute__((__always_inline__))
    #else
        #define FORCEINLINE
    #endif
#endif

// ---------------- Restrict ----------------

#if defined(_MSC_VER)
    #define RESTRICT __restrict
#else
    #define RESTRICT __restrict__
#endif

// ---------------- Debugging ----------------

#ifdef NDEBUG
    #define EXPECTS(X)
    #define ASSERT(X)
    #define ENSURES(X)
    #define EXPECTS_WITH_MESSAGE(X, ...)
    #define ASSERT_WITH_MESSAGE(X, ...)
    #define ENSURES_WITH_MESSAGE(X, ...)
#else
    #define EXPECTS(X) \
        if (!(X)) \
        { \
            std::cerr << "Precondition " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::terminate(); \
        }
    #define ASSERT(X) \
        if (!(X)) \
        { \
            std::cerr << "Assertion " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::terminate(); \
        }
    #define ENSURES(X) \
        if (!(X)) \
        { \
            std::cerr << "Postcondition " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::terminate(); \
        }
    #define EXPECTS_WITH_MESSAGE(X, ...) \
        if (!(X)) \
        { \
            std::cerr << "Precondition " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::cerr << "Error message : " << __VA_ARGS__ << std::endl; \
            std::terminate(); \
        }
    #define ASSERT_WITH_MESSAGE(X, ...) \
        if (!(X)) \
        { \
            std::cerr << "Assertion " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::cerr << "Error message : " << __VA_ARGS__ << std::endl; \
            std::terminate(); \
        }
    #define ENSURES_WITH_MESSAGE(X, ...) \
        if (!(X)) \
        { \
            std::cerr << "Postcondition " << AS_STRING(X) << " violated at " << __FILE__ << ":" << __LINE__ << "\n"; \
            std::cerr << "Error message : " << __VA_ARGS__ << std::endl; \
            std::terminate(); \
        }
#endif
