/**
 * @file Sentinel.hpp
 *
 * @brief Provides a generic sentinel type for various iterator types in itertools.
 */

#pragma once

#include <utility>

namespace enda::itertools
{
    /**
     * @brief Generic sentinel type that can be used to mark the end of a range.
     * @tparam Iter Iterator type.
     */
    template<typename Iter>
    struct sentinel_t
    {
        // End iterator of some range.
        Iter it;
    };

    /**
     * @brief Create an itertools::sentinel_t from an iterator using template type deduction.
     *
     * @tparam Iter Iterator type.
     * @param it Iterator to be turned into an itertools::sentinel_t.
     * @return Sentinel object.
     */
    template<typename Iter>
    [[nodiscard]] sentinel_t<Iter> make_sentinel(Iter it)
    {
        return {std::move(it)};
    }

} // namespace enda::itertools
