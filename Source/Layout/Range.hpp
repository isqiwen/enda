/**
 * @file Range.hpp
 *
 * @brief Includes the itertools header and provides some additional utilities.
 */

#pragma once

#include <ostream>
#include <type_traits>

#include "Itertools/Itertools.hpp"
#include "Traits.hpp"

namespace enda
{

    /**
     * @addtogroup layout_utils
     * @{
     */

    /// Using declaration for itertools::range.
    using itertools::range;

    /**
     * @brief Mimics Python's `...` syntax.
     *
     * @details While itertools's `range::all_t` mimics Python's `:`, `ellipsis` mimics Python's `...`. It is repeated as
     * much as necessary to match the number of dimensions of an array/view when used to access elements/slices.
     */
    struct ellipsis : range::all_t
    {};

    /**
     * @brief Write `enda::range::all_t` to a std::ostream as `_`.
     *
     * @param os Output stream.
     * @return Reference to the output stream.
     */
    inline std::ostream& operator<<(std::ostream& os, range::all_t) noexcept { return os << "_"; }

    /**
     * @brief Write enda::ellipsis to a std::ostream as `___`.
     *
     * @param os Output stream.
     * @return Reference to the output stream.
     */
    inline std::ostream& operator<<(std::ostream& os, ellipsis) noexcept { return os << "___"; }

    /// Constexpr variable that is true if the parameter pack `Args` contains an enda::ellipsis.
    template<typename... Args>
    constexpr bool ellipsis_is_present = is_any_of<ellipsis, std::remove_cvref_t<Args>...>;

    /**
     * @brief Constexpr variable that is true if the type `T` is either an `enda::range`, an `enda::range::all_t` or an
     * enda::ellipsis.
     */
    template<typename T>
    constexpr bool is_range_or_ellipsis = is_any_of<std::remove_cvref_t<T>, range, range::all_t, ellipsis>;

    /** @} */

} // namespace enda
