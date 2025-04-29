/**
 * @file Accessors.hpp
 *
 * @brief Defines accessors for enda::array objects (cf. std::default_accessor).
 */

#pragma once

#include <cstddef>

#include "Macros.hpp"

namespace enda
{
    struct default_accessor
    {
        template<typename T>
        struct accessor
        {
            // Value type of the data.
            using element_type = T;

            // Pointer type to the data.
            using pointer = T*;

            // Reference type to the data.
            using reference = T&;

            /**
             * @brief Access a specific element of the data.
             *
             * @param p Pointer to the data.
             * @param i Index of the element to access.
             * @return Reference to the element.
             */
            FORCEINLINE static reference access(pointer p, std::ptrdiff_t i) noexcept
            {
                EXPECTS(p != nullptr);
                return p[i];
            }

            /**
             * @brief Offset the pointer by a certain number of elements.
             *
             * @param p Pointer to the data.
             * @param i Number of elements to offset the pointer by.
             * @return Pointer after applying the offset.
             */
            FORCEINLINE static T* offset(pointer p, std::ptrdiff_t i) noexcept { return p + i; }
        };
    };

    struct no_alias_accessor
    {
        template<typename T>
        struct accessor
        {
            // Value type of the data.
            using element_type = T;

            // Restricted pointer type to the data.
            using pointer = T* RESTRICT;

            // Reference type to the data.
            using reference = T&;

            /**
             * @brief Access a specific element of the data.
             *
             * @param p Pointer to the data.
             * @param i Index of the element to access.
             * @return Reference to the element.
             */
            FORCEINLINE static reference access(pointer p, std::ptrdiff_t i) noexcept { return p[i]; }

            /**
             * @brief Offset the pointer by a certain number of elements.
             *
             * @param p Pointer to the data.
             * @param i Number of elements to offset the pointer by.
             * @return Pointer after applying the offset.
             */
            FORCEINLINE static T* offset(pointer p, std::ptrdiff_t i) noexcept { return p + i; }
        };
    };

} // namespace enda
