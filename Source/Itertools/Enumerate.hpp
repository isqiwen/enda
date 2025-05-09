/**
 * @file Enumerate.hpp
 *
 * @brief Provides a range adapting function for enumerating a given range/view.
 */

#pragma once

#include <iterator>
#include <tuple>
#include <utility>

#include "Itertools/IteratorFacade.hpp"
#include "Itertools/Sentinel.hpp"

namespace enda::itertools
{
    /**
     * @brief Iterator for an itertools::enumerated range.
     *
     * @details It stores an iterator of the original range and an index. Incrementing advances the iterator
     * and the index by 1. Dereferencing returns a std::pair consisting of the current index and the current
     * dereferenced value of the original iterator.
     *
     * See itertools::enumerate(R &&) for more details.
     *
     * @tparam Iter Iterator type.
     */
    template<typename Iter>
    struct enum_iter : iterator_facade<enum_iter<Iter>, std::pair<long, typename std::iterator_traits<Iter>::value_type>>
    {
        // Iterator of the original range.
        Iter it;

        // Index for enumerating.
        long i = 0;

        // Default constructor sets the index to zero and default constructs the original iterator.
        enum_iter() = default;

        /**
         * @brief Construct an enumerated iterator from a given iterator and set the index to zero.
         * @param it Iterator of the original range.
         */
        enum_iter(Iter it) : it(std::move(it)) {}

        // Increment the iterator by incrementing the original iterator and the index.
        void increment()
        {
            ++it;
            ++i;
        }

        /**
         * @brief Equal-to operator for two itertools::enum_iter objects.
         *
         * @param other itertools::enum_iter to compare with.
         * @return True, if the original iterators are equal.
         */
        [[nodiscard]] bool operator==(enum_iter const& other) const { return it == other.it; }

        /**
         * @brief Equal-to operator for a itertools::enum_iter and an itertools::sentinel_t.
         *
         * @tparam SentinelIter Iterator type of the sentinel.
         * @param s itertools::sentinel_t to compare with.
         * @return True, if the original iterator is equal to the iterator stored in the sentinel.
         */
        template<typename SentinelIter>
        [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const& s) const
        {
            return it == s.it;
        }

        /**
         * @brief Dereference the iterator.
         * @return Tuple consisting of the current index and the current dereferenced value of the original iterator.
         */
        [[nodiscard]] decltype(auto) dereference() const { return std::tuple<long, decltype(*it)> {i, *it}; }
    };

    /**
     * @brief Represents an enumerated range.
     *
     * @details See itertools::enumerate(R &&) for more details.
     *
     * @tparam R Range type.
     */
    template<typename R>
    struct enumerated
    {
        // Original range.
        R rg;

        // Iterator type of the enumerated range.
        using iterator = enum_iter<decltype(std::begin(rg))>;

        // Const iterator type of the enumerated range.
        using const_iterator = enum_iter<decltype(std::cbegin(rg))>;

        // Default equal-to operator.
        [[nodiscard]] bool operator==(enumerated const&) const = default;

        /**
         * @brief Beginning of the enumerated range.
         * @return itertools::enum_iter constructed from the beginning of the original range with the index set to zero.
         */
        [[nodiscard]] iterator begin() noexcept { return std::begin(rg); }

        // Const version of begin().
        [[nodiscard]] const_iterator cbegin() const noexcept { return std::cbegin(rg); }

        // Const overload of begin().
        [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

        /**
         * @brief End of the enumerated range.
         * @return itertools::sentinel_t containing the end iterator of the original range.
         */
        [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(rg)); }

        // Const version of end().
        [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(rg)); }

        // Const overload of end().
        [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    /**
     * @brief Lazy-enumerate a given range (similar to Python's enumerate).
     *
     * @details Each element in the original range is assigned an index, starting from zero. This function
     * returns an iterable lazy object (a itertools::enumerated range), which iterates over tuples consisting
     * of the index and the value of the dereferenced iterator of the original range:
     *
     * @code{.cpp}
     * std::vector<char> vec { 'a', 'b', 'c' };
     *
     * for (auto [idx, val] : enumerate(vec)) {
     *   std::cout << "(" << idx << ", " << val << ")\n";
     * }
     * @endcode
     *
     * Output:
     *
     * ```
     * (0, a)
     * (1, b)
     * (2, c)
     * ```
     *
     * See also <a href="https://en.cppreference.com/w/cpp/ranges/enumerate_view">std::ranges::views::enumerate</a>.
     *
     * @tparam R Range type.
     * @param rg Range to enumerate.
     * @return A itertools::enumerated range.
     */
    template<typename R>
    [[nodiscard]] enumerated<R> enumerate(R&& rg)
    {
        return {std::forward<R>(rg)};
    }

} // namespace enda::itertools
