/**
 * @file Product.hpp
 *
 * @brief Provides a range adapting function for multiplying a given number of ranges/views (cartesian product).
 */

#pragma once

#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>

#include "Itertools/IteratorFacade.hpp"
#include "Itertools/Sentinel.hpp"

namespace enda::itertools
{
    /**
     * @brief Iterator for a itertools::multiplied (cartesian product) range.
     *
     * @details It stores three tuples of iterators of the original ranges:
     * - `its_begin` contains the begin iterators of all ranges
     * - `its_end` contains the end iterators of all ranges
     * - `its` contains the current iterators of all ranges
     *
     * Incrementing is done from right to left, i.e. the iterator of the last range is incremented first.
     * Once an iterator reaches the end of its range, it is reset to the beginning and the iterator of the
     * previous range is incremented once.
     *
     * Dereferencing returns a tuple containing the results of dereferencing each iterator.
     *
     * See itertools::product(Rs &&...rgs) for more details.
     *
     * @tparam EndIters Tuple type containing the end iterators of all ranges.
     * @tparam Iters Iterator types.
     */
    template<typename EndIters, typename... Iters>
    struct prod_iter : iterator_facade<prod_iter<EndIters, Iters...>, std::tuple<typename std::iterator_traits<Iters>::value_type...>>
    {
        // Tuple containing the begin iterators of the original ranges.
        std::tuple<Iters...> its_begin;

        // Tuple containing the end iterators of the original ranges.
        EndIters its_end;

        // Tuple containing the current iterators of the original ranges.
        std::tuple<Iters...> its = its_begin;

        // Number of original ranges.
        static constexpr long Rank = sizeof...(Iters);

        // Default constructor.
        prod_iter() = default;

        /**
         * @brief Construct a product iterator from given begin iterators and end iterators.
         *
         * @param its_begin Tuple containing begin iterators of the original ranges.
         * @param its_end Tuple containing end iterators of the original ranges.
         */
        prod_iter(std::tuple<Iters...> its_begin, EndIters its_end) : its_begin(std::move(its_begin)), its_end(std::move(its_end)) {}

    private:
        // Helper function to recursively increment the current iterators.
        template<int N>
        void _increment()
        {
            // increment Nth iterator
            ++std::get<N>(its);
            // recursively increment previous iterators if necessary
            if constexpr (N > 0)
            {
                // if Nth iterator is at its end, reset it to its begin iterator and increment N-1st iterator
                if (std::get<N>(its) == std::get<N>(its_end))
                {
                    std::get<N>(its) = std::get<N>(its_begin);
                    _increment<N - 1>();
                }
            }
        }

    public:
        // Increment the iterator by incrementing the current iterators starting with the iterator of the last range.
        void increment() { _increment<Rank - 1>(); }

        /**
         * @brief Equal-to operator for two itertools::prod_iter objects.
         *
         * @param other itertools::prod_iter to compare with.
         * @return True, if all original iterators are equal.
         */
        [[nodiscard]] bool operator==(prod_iter const& other) const { return its == other.its; }

        /**
         * @brief Equal-to operator for a itertools::prod_iter and an itertools::sentinel_t.
         *
         * @details We reach the end of the product range, when the first iterator, i.e. `std::get<0>(its)`, is at its end.
         *
         * @tparam SentinelIter Iterator type of the sentinel.
         * @param s itertools::sentinel_t to compare with.
         * @return True, if the first iterator, i.e. `std::get<0>(its)`, is equal to the iterator of the sentinel.
         */
        template<typename SentinelIter>
        [[nodiscard]] bool operator==(sentinel_t<SentinelIter> const& s) const
        {
            return (s.it == std::get<0>(its));
        }

    private:
        // Helper function to dereference all original iterators.
        template<size_t... Is>
        FORCEINLINE [[nodiscard]] auto tuple_map_impl(std::index_sequence<Is...>) const
        {
            return std::tuple<decltype(*std::get<Is>(its))...>(*std::get<Is>(its)...);
        }

    public:
        /**
         * @brief Dereference the iterator.
         * @return Tuple containing the dereferenced values of all original iterators.
         */
        [[nodiscard]] decltype(auto) dereference() const { return tuple_map_impl(std::index_sequence_for<Iters...> {}); }
    };

    /**
     * @ingroup adapted_ranges
     * @brief Represents a cartesian product of ranges.
     *
     * @details See itertools::product(Rs &&...rgs) for more details.
     *
     * @tparam Rs Range types.
     */
    template<typename... Rs>
    struct multiplied
    {
        // Tuple containing the original ranges.
        std::tuple<Rs...> tu;

        // Iterator type of the product range.
        using iterator = prod_iter<std::tuple<decltype(std::end(std::declval<Rs&>()))...>, decltype(std::begin(std::declval<Rs&>()))...>;

        // Const iterator type the product range.
        using const_iterator = prod_iter<std::tuple<decltype(std::cend(std::declval<Rs&>()))...>, decltype(std::cbegin(std::declval<Rs&>()))...>;

        /**
         * @brief Constructs a cartesian product (multiplied) range from the given ranges.
         *
         * @tparam Us Range types.
         * @param rgs Ranges to be multiplied.
         */
        template<typename... Us>
        multiplied(Us&&... rgs) : tu {std::forward<Us>(rgs)...}
        {}

        // Default equal-to operator.
        [[nodiscard]] bool operator==(multiplied const&) const = default;

    private:
        // Helper function to create a itertools::prod_iter representing the beginning of the product range.
        template<size_t... Is>
        FORCEINLINE auto _begin(std::index_sequence<Is...>)
        {
            return iterator {std::make_tuple(std::begin(std::get<Is>(tu))...), std::make_tuple(std::end(std::get<Is>(tu))...)};
        }

        // Const version of _begin(std::index_sequence<Is...>).
        template<size_t... Is>
        FORCEINLINE auto _cbegin(std::index_sequence<Is...>) const
        {
            return const_iterator {std::make_tuple(std::cbegin(std::get<Is>(tu))...), std::make_tuple(std::cend(std::get<Is>(tu))...)};
        }

    public:
        /**
         * @brief Beginning of the product range.
         * @return itertools::prod_iter representing the beginning of the product range.
         */
        [[nodiscard]] iterator begin() noexcept { return _begin(std::index_sequence_for<Rs...> {}); }

        // Const version of begin().
        [[nodiscard]] const_iterator cbegin() const noexcept { return _cbegin(std::index_sequence_for<Rs...> {}); }

        // Const overload of begin().
        [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }

        /**
         * @brief End of the product range.
         * @return itertools::sentinel_t containing the end iterator of the first original range, i.e. `std::end(std::get<0>(tu))`.
         */
        [[nodiscard]] auto end() noexcept { return make_sentinel(std::end(std::get<0>(tu))); }

        // Const version of end().
        [[nodiscard]] auto cend() const noexcept { return make_sentinel(std::cend(std::get<0>(tu))); }

        // Const overload of end().
        [[nodiscard]] auto end() const noexcept { return cend(); }
    };

    // Class template argument deduction guide.
    template<typename... Rs>
    multiplied(Rs&&...) -> multiplied<std::decay_t<Rs>...>;

    /**
     * @addtogroup range_adapting_functions
     * @{
     */

    /**
     * @brief Lazy-multiply a given number of ranges by forming their cartesian product.
     *
     * @details An arbitrary number of ranges are multiplied together into a cartesian product range.
     * They are traversed such that the last range is traversed the fastest (see the example below).
     * The number of elements in a product range is equal to the product of the sizes of the given ranges.
     * This function returns an iterable lazy object, which can be used in range-based for loops:
     *
     * @code{.cpp}
     * std::vector<int> v1 { 1, 2, 3 };
     * std::vector<char> v2 { 'a', 'b' };
     *
     * for (auto [i, c] : product(v1, v2)) {
     *   std::cout << "(" << i << ", " << c << ")\n";
     * }
     * @endcode
     *
     * Output:
     *
     * ```
     * (1, a)
     * (1, b)
     * (2, a)
     * (2, b)
     * (3, a)
     * (3, b)
     * ```
     *
     * See also <a href="https://en.cppreference.com/w/cpp/ranges/cartesian_product_view">std::ranges::views::cartesian_product</a>.
     *
     * @tparam Rs Range types.
     * @param rgs Ranges to be used.
     * @return A product (itertools::multiplied) range.
     */
    template<typename... Rs>
    [[nodiscard]] itertools::multiplied<Rs...> product(Rs&&... rgs)
    {
        return {std::forward<Rs>(rgs)...};
    }

    namespace detail
    {

        // Helper function to create a product range from a container of ranges.
        template<typename C, size_t... Is>
        FORCEINLINE [[nodiscard]] auto make_product_impl(C& cont, std::index_sequence<Is...>)
        {
            return product(cont[Is]...);
        }

    } // namespace detail

    /**
     * @brief Create a cartesian product range from an array of ranges.
     *
     * @tparam R Range type.
     * @tparam N Number of ranges.
     * @param arr Array of ranges.
     * @return A product (itertools::multiplied) range from the ranges in the array.
     */
    template<typename R, size_t N>
    [[nodiscard]] auto make_product(std::array<R, N>& arr)
    {
        return detail::make_product_impl(arr, std::make_index_sequence<N> {});
    }

    // Const overload of make_product(std::array<R, N> &).
    template<typename R, size_t N>
    [[nodiscard]] auto make_product(std::array<R, N> const& arr)
    {
        return detail::make_product_impl(arr, std::make_index_sequence<N> {});
    }

} // namespace enda::itertools
