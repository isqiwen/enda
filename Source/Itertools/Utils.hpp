/**
 * @file Utils.hpp
 *
 * @brief Provides some utility functions for itertools.
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

namespace enda::itertools
{
    /**
     * @brief Advance an iterator by a given number of steps, up to a specified sentinel.
     *
     * @details Similar to <a href="https://en.cppreference.com/w/cpp/iterator/advance">std::advance</a>, this function
     * attempts to advance the given iterator by n steps. If the iterator reaches the sentinel (end of the range)
     * before advancing n steps, the iterator is set to the sentinel and the function returns the number of steps
     * that could not be advanced (i.e., the deficit).
     *
     * For random access iterators, the function operates in O(1) time using subtraction and addition. For other
     * iterator categories, the function advances the iterator step-by-step in O(n) time.
     *
     * @tparam Iter Iterator type.
     * @tparam Sentinel Type of the sentinel (end iterator).
     * @param it Iterator to be advanced (passed by reference).
     * @param n Number of steps to advance the iterator.
     * @param end Sentinel representing the end of the range.
     * @return The number of steps that could not be advanced (0 if the iterator was successfully advanced by n steps).
     */
    template<typename Iter>
    auto advance(Iter& it, typename std::iterator_traits<Iter>::difference_type n, Iter end) -> typename std::iterator_traits<Iter>::difference_type
    {
        using Diff = typename std::iterator_traits<Iter>::difference_type;

        if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
        {
            // For random access iterators, we can compute the distance in O(1)
            Diff available = end - it;
            if (available < n)
            {
                it = end;
                return n - available; // missing steps = n - steps advanced
            }
            else
            {
                it += n;
                return 0;
            }
        }
        else
        {
            // For non-random access iterators, we iterate one by one.
            Diff missing = n;
            while (missing > 0 && it != end)
            {
                ++it;
                --missing;
            }
            return missing;
        }
    }

    /**
     * @brief Calculate the distance between two iterators.
     *
     * @details It is similar to <a href="https://en.cppreference.com/w/cpp/iterator/distance">std::distance</a>,
     * except that it can be used for two different iterator types, e.g. in case one of them is a const iterator.
     *
     * @tparam Iter1 Iterator type #1.
     * @tparam Iter2 Iterator type #2.
     * @param first Iterator #1.
     * @param last Iterator #2.
     * @return Number of elements between the two iterators.
     */
    template<typename Iter1, typename Iter2>
    [[nodiscard]] inline typename std::iterator_traits<Iter1>::difference_type distance(Iter1 first, Iter2 last)
    {
        // O(1) for random access iterators
        if constexpr (std::is_same_v<typename std::iterator_traits<Iter1>::iterator_category, std::random_access_iterator_tag>)
        {
            return last - first;
        }
        else
        { // O(n) for other iterators
            typename std::iterator_traits<Iter1>::difference_type r(0);
            for (; first != last; ++first)
                ++r;
            return r;
        }
    }

    /**
     * @brief Create a vector from a range.
     *
     * @tparam R Range type.
     * @param rg Range.
     * @return std::vector<T> containing the elements of the range, where T denotes the value type of the range.
     */
    template<typename R>
    [[nodiscard]] auto make_vector_from_range(R const& rg)
    {
        std::vector<std::decay_t<decltype(*(std::begin(rg)))>> vec {};
        // do we really want to reserve memory here? maybe only for random access ranges?
        if constexpr (std::is_same_v<decltype(std::cbegin(rg)), decltype(std::cend(rg))>)
        {
            auto total_size = distance(std::cbegin(rg), std::cend(rg));
            vec.reserve(total_size);
        }
        for (auto const& x : rg)
            vec.emplace_back(x);
        return vec;
    }

    /**
     * @brief Given an integer range `[first, last)`, divide it as equally as possible into N chunks.
     *
     * @details It is intended to divide a range among different processes. If the size of the range is not
     * divisible by N without a remainder, i.e. `r = (last - first) % N`, then the first `r` chunks have one more element.
     *
     * @param first First value of the range.
     * @param last Last value of the range (excluded).
     * @param n_chunks Number of chunks to divide the range into.
     * @param rank Rank of the calling process.
     * @return Pair of indices specifying the first and last (excluded) value of the chunk assigned to the calling process.
     */
    [[nodiscard]] inline std::pair<std::ptrdiff_t, std::ptrdiff_t> chunk_range(std::ptrdiff_t first, std::ptrdiff_t last, long n_chunks, long rank)
    {
        auto total_size    = last - first;
        auto chunk_size    = total_size / n_chunks;
        auto n_large_nodes = total_size - n_chunks * chunk_size;
        if (rank < n_large_nodes)
            return {first + rank * (chunk_size + 1), first + (rank + 1) * (chunk_size + 1)};
        else
            return {first + n_large_nodes + rank * chunk_size, first + n_large_nodes + (rank + 1) * chunk_size};
    }

} // namespace enda::itertools
