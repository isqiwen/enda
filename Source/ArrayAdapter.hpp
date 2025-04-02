/**
 * @file ArrayAdapter.hpp
 *
 * @brief Provides an array adapter class.
 */

#pragma once

#include <array>
#include <type_traits>

#include "Concepts.hpp"
#include "StdUtil/Array.hpp"

namespace enda
{

    /**
     * @addtogroup av_utils
     * @{
     */

    /**
     * @brief Adapter that consists of a shape and a callable object, which takes `R` integers as arguments (just like an
     * enda::basic_array or enda::basic_array_view).
     *
     * @tparam R Rank of the adapter.
     * @tparam F Callable type.
     */
    template<int R, typename F>
    class array_adapter
    {
        static_assert(CallableWithLongs<F, R>, "Error in enda::array_adapter: Lambda should be callable with R integers");

        // Shape of the array.
        std::array<long, R> myshape;

        // Callable object.
        F f;

    public:
        /**
         * @brief Construct a new array adapter object.
         *
         * @tparam Int Integer type.
         * @param shape Shape of the adapater.
         * @param f Callable object.
         */
        template<typename Int>
        array_adapter(std::array<Int, R> const& shape, F f) : myshape(stdutil::make_std_array<long>(shape)), f(f)
        {}

        /**
         * @brief Get shape of the adapter.
         * @return `std::array<long, R>` object specifying the shape of the adapter.
         */
        [[nodiscard]] auto const& shape() const { return myshape; }

        /**
         * @brief Get the total size of the adapter.
         * @return Number of elements in the adapter.
         */
        [[nodiscard]] long size() const { return stdutil::product(myshape); }

        /**
         * @brief Function call operator simply forwards the arguments to the callable object.
         *
         * @tparam Ints Integer types (convertible to long).
         * @param i0 First argument.
         * @param is Rest of the arguments.
         */
        template<typename... Ints>
        auto operator()(long i0, Ints... is) const
        {
            static_assert((std::is_convertible_v<Ints, long> and ...), "Error in enda::array_adapter: Arguments must be convertible to long");
            return f(i0, is...);
        }
    };

    // Class template argument deduction guides.
    template<auto R, typename Int, typename F>
    array_adapter(std::array<Int, R>, F) -> array_adapter<R, F>;

    /** @} */

    /**
     * @ingroup utils_type_traits
     * @brief Specialization of enda::get_algebra for enda::array_adapter.
     */
    template<int R, typename F>
    inline constexpr char get_algebra<array_adapter<R, F>> = 'A';

} // namespace enda
