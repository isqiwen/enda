/**
 * @file Map.hpp
 *
 * @brief Provides lazy function calls on arrays/views.
 */

#pragma once

#include <cstddef>
#include <tuple>
#include <utility>

#include "Concepts.hpp"
#include "Layout/Range.hpp"
#include "Macros.hpp"
#include "Traits.hpp"

namespace enda
{

    /// @cond
    // Forward declarations.
    template<typename F, Array... A>
    struct expr_call;

    template<class F>
    struct mapped;
    /// @endcond

    namespace detail
    {

        // Implementation of the enda::get_algebra trait for function call expressions.
        template<typename... Char>
        constexpr char _impl_find_common_algebra(char x0, Char... x)
        {
            return (((x == x0) && ...) ? x0 : 'N');
        }

    } // namespace detail

    /**
     * @ingroup av_utils
     * @brief Get the resulting algebra of a function call expression involving arrays/views.
     *
     * @details If one of the algebras of the arguments is different, the resulting algebra is 'N'.
     *
     * @tparam F Callable object of the expression.
     * @tparam As enda::Array argument types.
     */
    template<typename F, Array... As>
    constexpr char get_algebra<expr_call<F, As...>> = detail::_impl_find_common_algebra(get_algebra<As>...);

    /**
     * @addtogroup av_math
     * @{
     */

    /**
     * @brief A lazy function call expression on arrays/views.
     *
     * @details The lazy expression call fulfils the enda::Array concept and can therefore be assigned to other
     * enda::basic_array or enda::basic_array_view objects. For example:
     *
     * @code{.cpp}
     * enda::matrix<int> mat{{1, 2}, {3, 4}};
     * enda::matrix<int> pmat = enda::pow(mat, 2);
     * @endcode
     *
     * Here, `enda::pow(mat, 2)` returns a lazy expression call object which is then used in the constructor of `pmat`.
     *
     * The callable object should take the array/view elements as arguments.
     *
     * @tparam F Callable type.
     * @tparam As enda::Array argument types.
     */
    template<typename F, Array... As>
    struct expr_call
    {
        /// Callable object of the expression.
        F f;

        /// Tuple containing the enda::Array arguments.
        std::tuple<const As...> a;

    private:
        // Implementation of the function call operator.
        template<size_t... Is, typename... Args>
        [[gnu::always_inline]] [[nodiscard]] auto _call(std::index_sequence<Is...>, Args const&... args) const
        {
            // if args contains a range, we need to return an expr_call on the resulting slice
            if constexpr ((is_range_or_ellipsis<Args> or ... or false))
            {
                return mapped<F> {f}(std::get<Is>(a)(args...)...);
            }
            else
            {
                return f(std::get<Is>(a)(args...)...);
            }
        }

        // Implementation of the subscript operator.
        template<size_t... Is, typename Arg>
        [[gnu::always_inline]] auto _call_bra(std::index_sequence<Is...>, Arg const& arg) const
        {
            return f(std::get<Is>(a)[arg]...);
        }

    public:
        /**
         * @brief Function call operator.
         *
         * @details The arguments (usually multi-dimensional indices) are passed to all the enda::Array objects stored in the
         * tuple and the results are then passed to the callable object.
         *
         * If the arguments contain a range, a new lazy function call expression is returned.
         *
         * @tparam Args Argument types.
         * @param args Function call arguments.
         * @return The result of the function call (depends on the callable and the arguments).
         */
        template<typename... Args>
        auto operator()(Args const&... args) const
        {
            return _call(std::make_index_sequence<sizeof...(As)> {}, args...);
        }

        /**
         * @brief Subscript operator.
         *
         * @details The argument (usually a 1-dimensional index) is passed to all the enda::Array objects stored in the tuple
         * and the results are then passed to the callable object.
         *
         * If the argument is a range, a new lazy function call expression is returned.
         *
         * @tparam Arg Argument types.
         * @param arg Subscript argument.
         * @return The result of the subscript operation (depends on the callable and the arguments).
         */
        template<typename Arg>
        auto operator[](Arg const& arg) const
        {
            return _call_bra(std::make_index_sequence<sizeof...(As)> {}, arg);
        }

        // FIXME copy needed for the && case only. Overload ?
        /**
         * @brief Get the shape of the enda::Array objects.
         * @return `std::array<long, Rank>` object specifying the shape of each enda::Array object.
         */
        [[nodiscard]] auto shape() const { return std::get<0>(a).shape(); }

        /**
         * @brief Get the total size of the enda::Array objects.
         * @return Number of elements contained in each enda::Array object.
         */
        [[nodiscard]] long size() const { return std::get<0>(a).size(); }
    };

    /**
     * @brief Functor that is returned by the enda::map function.
     * @tparam F Callable type.
     */
    template<class F>
    struct mapped
    {
        /// Callable object.
        F f;

        /**
         * @brief Function call operator that returns a lazy function call expression.
         *
         * @tparam A0 First enda::Array argument type.
         * @tparam As Rest of the enda::Array argument types.
         * @param a0 First enda::Array argument.
         * @param as Rest of the enda::Array arguments.
         * @return A lazy enda::expr_call object.
         */
        template<Array A0, Array... As>
        expr_call<F, A0, As...> operator()(A0&& a0, As&&... as) const
        {
            EXPECTS(((as.shape() == a0.shape()) && ...)); // same shape
            return {f, {std::forward<A0>(a0), std::forward<As>(as)...}};
        }
    };

    /**
     * @brief Create a lazy function call expression on arrays/views.
     *
     * @details The callable should take the array/view elements as arguments.
     *
     * @tparam F Callable type.
     * @param f Callable object.
     * @return A lazy enda::mapped object.
     */
    template<class F>
    mapped<F> map(F f)
    {
        return {std::move(f)};
    }

    /** @} */

} // namespace enda
