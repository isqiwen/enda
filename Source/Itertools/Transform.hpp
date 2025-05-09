/**
 * @file Transform.hpp
 *
 * @brief Provides a range adapting function for transforming a given range/view.
 */

#pragma once

#include <iterator>
#include <optional>
#include <type_traits>
#include <utility>

#include "Itertools/IteratorFacade.hpp"
#include "Itertools/Sentinel.hpp"

namespace enda::itertools
{
    /**
     * @brief Iterator for a itertools::transformed range.
     *
     * @details It stores an iterator of the original range and a callable object that is used to transform the
     * elements of the original range. Incrementing simply increments the iterator. Dereferencing returns the
     * result of the callable object applied to the dereferenced iterator, i.e. the transformed element.
     *
     * See itertools::transform(R &&, F) for more details.
     *
     * @tparam Iter Iterator type.
     * @tparam F Callable type.
     * @tparam Value Return type of the callable.
     */
    template<typename Iter, typename F, typename Value = std::invoke_result_t<F, typename std::iterator_traits<Iter>::value_type>>
    struct transform_iter : iterator_facade<transform_iter<Iter, F>, Value>
    {
        // Iterator of the original range.
        Iter it;

        // Callable doing the transformation.
        mutable std::optional<F> lambda;

        // Default constructor.
        transform_iter() = default;

        /**
         * @brief Construct a transformed iterator from a given iterator and callable.
         *
         * @param it Iterator of the original range.
         * @param lambda Callable doing the transformation.
         */
        transform_iter(Iter it, F lambda) : it(std::move(it)), lambda(std::move(lambda)) {}

        // Increment the iterator by incrementing the original iterator.
        void increment() { ++it; }

        // Default move constructor.
        transform_iter(transform_iter&&) = default;

        // Default copy constructor.
        transform_iter(transform_iter const&) = default;

        // Default move assignment operator.
        transform_iter& operator=(transform_iter&&) = default;

        /**
         * @brief Custom copy assignment operator makes sure that the optional callable is correctly copied.
         * @param other itertools::transform_iter to copy from.
         */
        transform_iter& operator=(transform_iter const& other)
        {
            it = other.it;
            if (other.lambda.has_value())
                lambda.emplace(other.lambda.value());
            else
                lambda.reset();
            return *this;
        }

        /**
         * @brief Equal-to operator for two itertools::transform_iter objects.
         *
         * @param other itertools::transform_iter to compare with.
         * @return True, if the original iterators are equal.
         */
        [[nodiscard]] bool operator==(transform_iter const& other) const { return it == other.it; }

        /**
         * @brief Equal-to operator for a itertools::transform_iter and an itertools::sentinel_t.
         *
         * @tparam SentinelIter Iterator type of the sentinel.
         * @param s itertools::sentinel_t to compare with.
         * @return True, if the original iterator is equal to the iterator stored in the sentinel.
         */
        template<typename SentinelIter>
        [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const& s) const
        {
            return (it == s.it);
        }

        /**
         * @brief Dereference the iterator.
         * @return Result of the callable applied to the dereferenced iterator, i.e. the transformed value.
         */
        [[nodiscard]] decltype(auto) dereference() const { return (*lambda)(*it); }
    };

    /**
     * @ingroup adapted_ranges
     * @brief Represents a transformed range.
     *
     * @details See itertools::transform(R &&, F) for more details.
     *
     * @tparam R Range type.
     * @tparam F Callable type.
     */
    template<typename R, typename F>
    struct transformed
    {
        // Original range.
        R rg;

        // Callable doing the transformation.
        F lambda;

        // Const iterator type of the transformed range.
        using const_iterator = transform_iter<decltype(std::cbegin(rg)), F>;

        // Iterator type of the transformed range.
        using iterator = const_iterator;

        /**
         * @brief Beginning of the transformed range.
         * @return itertools::transform_iter constructed from the beginning of the original range and the callable.
         */
        [[nodiscard]] const_iterator cbegin() const noexcept { return {std::cbegin(rg), lambda}; }

        // The same as cbegin().
        [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

        /**
         * @brief End of the transformed range.
         * @return itertools::sentinel_t containing the end iterator of the original range.
         */
        [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(rg)); }

        // The same as cend().
        [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    /**
     * @ingroup range_adapting_functions
     * @brief Lazy-transform a given range by applying a unary callable object to every element of the original range.
     *
     * @details The value type of the transformed range depends on the return type of the callable.
     *
     * This function returns an iterable lazy object (a itertools::transformed range), which can be used in range-based for loops:
     *
     * @code{.cpp}
     * std::list<int> list { 1, 2, 3, 4, 5 };
     *
     * for (auto i : itertools::transform(list, [](int i) { return i * i; })) {
     *   std::cout << i << " ";
     * }
     * @endcode
     *
     * Output:
     *
     * ```
     * 1 4 9 16 25
     * ```
     *
     * See also <a href="https://en.cppreference.com/w/cpp/ranges/transform_view">std::ranges::views::transform</a>.
     *
     * @tparam R Range type.
     * @tparam F Callable type.
     * @param rg Range to transform.
     * @param lambda Callable to be applied to the given range.
     * @return A itertools::transformed range.
     */
    template<typename R, typename F>
    [[nodiscard]] auto transform(R&& rg, F lambda)
    {
        return transformed<R, F> {std::forward<R>(rg), std::move(lambda)};
    }

} // namespace enda::itertools
