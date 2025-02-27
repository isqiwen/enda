#pragma once

#include <cassert> // for assert
#include <version> // for __cpp_lib_unreachable, ...

/**
 * @file macros.hpp
 *
 * @brief A collection of internal/public macros.
 *
 * These are exhaustively documented due to macros nasty visibility rules however, only
 * macros that are marked as __[public]__ should be consumed.
 */

// NOLINTBEGIN Sometime macros are the only way to do things...

/**
 * @brief __[public]__ The major version of libfork.
 *
 * Increments with incompatible API changes.
 */
#define ENDA_VERSION_MAJOR 1
/**
 * @brief __[public]__ The minor version of libfork.
 *
 * Increments when functionality is added in an API backward compatible manner.
 */
#define ENDA_VERSION_MINOR 8
/**
 * @brief __[public]__ The patch version of libfork.
 *
 * Increments when bug fixes are made in an API backward compatible manner.
 */
#define ENDA_VERSION_PATCH 0

/**
 * @brief Use to conditionally decorate lambdas and ``operator()`` (alongside ``LF_STATIC_CONST``) with
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

#ifdef __cpp_lib_unreachable
    #include <utility> // for std::unreachable
#endif

namespace enda
{

#ifdef __cpp_lib_unreachable
    using std::unreachable;
#else
    /**
     * @brief A homebrew version of `std::unreachable`, see https://en.cppreference.com/w/cpp/utility/unreachable
     */
    [[noreturn]] inline void unreachable()
    {
    // Uses compiler specific extensions if possible.
    #if defined(_MSC_VER) && !defined(__clang__) // MSVC
        __assume(false);
    #else                                        // GCC, Clang
        __builtin_unreachable();
    #endif
        // Even if no extension is used, undefined behavior is still raised by infinite loop.
        for (;;)
        {
        }
    }
#endif

} // namespace enda

/**
 * @brief Invokes undefined behavior if ``expr`` evaluates to `false`.
 *
 * \rst
 *
 *  .. warning::
 *
 *    This has different semantics than ``[[assume(expr)]]`` as it WILL evaluate the
 *    expression at runtime. Hence you should conservatively only use this macro
 *    if ``expr`` is side-effect free and cheap to evaluate.
 *
 * \endrst
 */

#define ENDA_ASSUME(expr) \
    do \
    { \
        if (!(expr)) \
        { \
            ::lf::impl::unreachable(); \
        } \
    } while (false)

/**
 * @brief If ``NDEBUG`` is defined then ``LF_ASSERT(expr)`` is  `` `` otherwise ``assert(expr)``.
 *
 * This is for expressions with side-effects.
 */
#ifndef NDEBUG
    #define ENDA_ASSERT_NO_ASSUME(expr) assert(expr)
#else
    #define ENDA_ASSERT_NO_ASSUME(expr) \
        do \
        { \
        } while (false)
#endif

/**
 * @brief If ``NDEBUG`` is defined then ``ENDA_ASSERT(expr)`` is  ``ENDA_ASSUME(expr)`` otherwise
 * ``assert(expr)``.
 */
#ifndef NDEBUG
    #define ENDA_ASSERT(expr) assert(expr)
#else
    #define ENDA_ASSERT(expr) ENDA_ASSUME(expr)
#endif

#ifndef _CCQ_MACROS_GUARD_H
    #define _CCQ_MACROS_GUARD_H

// CCQ, TRIQS general macros
// GUARD IT do not use pragma once
// hence one can simply include them in every project

// ---------------- Stringify ----------------

    #define AS_STRING(...) AS_STRING2(__VA_ARGS__)
    #define AS_STRING2(...) #__VA_ARGS__

// ---------------- Print ----------------

    #define PRINT(X) std::cerr << AS_STRING(X) << " = " << X << "      at " << __FILE__ << ":" << __LINE__ << '\n'
    #define NDA_PRINT(X) std::cerr << AS_STRING(X) << " = " << X << "      at " << __FILE__ << ":" << __LINE__ << '\n'

// ---------------- Inline ----------------

    #define FORCEINLINE __inline__ __attribute__((always_inline))

// ---------------- Debugging ----------------

    #ifdef NDEBUG

        #define EXPECTS(X)
        #define ASSERT(X)
        #define ENSURES(X)
        #define EXPECTS_WITH_MESSAGE(X, ...)
        #define ASSERT_WITH_MESSAGE(X, ...)
        #define ENSURES_WITH_MESSAGE(X, ...)

    #else

        #include <exception>
        #include <iostream>

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

    #endif // NDEBUG

#endif // _CCQ_MACROS_GUARD_H
