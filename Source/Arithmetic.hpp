/**
 * @file Arithmetic.hpp
 *
 * @brief Provides lazy expressions for enda::Array types.
 */

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

#include "Concepts.hpp"
#include "Declarations.hpp"
#include "Macros.hpp"
#include "StdUtil/Complex.hpp"
#include "Traits.hpp"

#ifdef ENDA_ENFORCE_BOUNDCHECK
    #include "Exceptions.hpp"
#endif // ENDA_ENFORCE_BOUNDCHECK

namespace enda
{
    template<char OP, Array A>
    struct expr_unary
    {
        static_assert(OP == '-', "Error in enda::expr_unary: Only negation is supported");

        // enda::Array object.
        A a;

        template<typename... Args>
        auto operator()(Args&&... args) const
        {
            return -a(std::forward<Args>(args)...);
        }

        [[nodiscard]] constexpr auto shape() const { return a.shape(); }

        [[nodiscard]] constexpr long size() const { return a.size(); }
    };

    template<char OP, ArrayOrScalar L, ArrayOrScalar R>
    struct expr
    {
        // enda::ArrayOrScalar left hand side operand.
        L l;

        // enda::ArrayOrScalar right hand side operand.
        R r;

        // Decay type of the left hand side operand.
        using L_t = std::decay_t<L>;

        // Decay type of the right hand side operand.
        using R_t = std::decay_t<R>;

        // Constexpr variable that is true if the left hand side operand is a scalar.
        static constexpr bool l_is_scalar = enda::is_scalar_v<L>;

        // Constexpr variable that is true if the right hand side operand is a scalar.
        static constexpr bool r_is_scalar = enda::is_scalar_v<R>;

        // Constexpr variable specifying the algebra of one of the non-scalar operands.
        static constexpr char algebra = (l_is_scalar ? get_algebra<R> : get_algebra<L>);

        static constexpr layout_info_t compute_layout_info()
        {
            if (l_is_scalar)
                return (algebra == 'A' ? get_layout_info<R> : layout_info_t {}); // 1 as an array has all flags, it is just 1
            if (r_is_scalar)
                return (algebra == 'A' ? get_layout_info<L> : layout_info_t {}); // 1 as a matrix does not, as it is diagonal only.
            return get_layout_info<R> & get_layout_info<L>;                      // default case. Take the logical and of all flags
        }

        [[nodiscard]] constexpr decltype(auto) shape() const
        {
            if constexpr (l_is_scalar)
            {
                return r.shape();
            }
            else if constexpr (r_is_scalar)
            {
                return l.shape();
            }
            else
            {
                EXPECTS(l.shape() == r.shape());
                return l.shape();
            }
        }

        [[nodiscard]] constexpr long size() const
        {
            if constexpr (l_is_scalar)
            {
                return r.size();
            }
            else if constexpr (r_is_scalar)
            {
                return l.size();
            }
            else
            {
                EXPECTS(l.size() == r.size());
                return l.size();
            }
        }

        template<typename... Args>
        auto operator()(Args const&... args) const
        {
            // addition
            if constexpr (OP == '+')
            {
                if constexpr (l_is_scalar)
                {
                    // lhs is a scalar
                    if constexpr (algebra == 'M')
                        // rhs is a matrix
                        return (std::equal_to {}(args...) ? l + r(args...) : r(args...));
                    else
                        // rhs is an array
                        return l + r(args...);
                }
                else if constexpr (r_is_scalar)
                {
                    // rhs is a scalar
                    if constexpr (algebra == 'M')
                        // lhs is a matrix
                        return (std::equal_to {}(args...) ? l(args...) + r : l(args...));
                    else
                        // lhs is an array
                        return l(args...) + r;
                }
                else
                    // both are arrays or matrices
                    return l(args...) + r(args...);
            }

            // subtraction
            if constexpr (OP == '-')
            {
                if constexpr (l_is_scalar)
                {
                    // lhs is a scalar
                    if constexpr (algebra == 'M')
                        // rhs is a matrix
                        return (std::equal_to {}(args...) ? l - r(args...) : -r(args...));
                    else
                        // rhs is an array
                        return l - r(args...);
                }
                else if constexpr (r_is_scalar)
                {
                    // rhs is a scalar
                    if constexpr (algebra == 'M')
                        // lhs is a matrix
                        return (std::equal_to {}(args...) ? l(args...) - r : l(args...));
                    else
                        // lhs is an array
                        return l(args...) - r;
                }
                else
                    // both are arrays or matrices
                    return l(args...) - r(args...);
            }

            // multiplication
            if constexpr (OP == '*')
            {
                if constexpr (l_is_scalar)
                    // lhs is a scalar
                    return l * r(args...);
                else if constexpr (r_is_scalar)
                    // rhs is a scalar
                    return l(args...) * r;
                else
                {
                    // both are arrays (matrix product is not supported here)
                    static_assert(algebra != 'M', "Error in enda::expr: Matrix algebra not supported");
                    return l(args...) * r(args...);
                }
            }

            // division
            if constexpr (OP == '/')
            {
                if constexpr (l_is_scalar)
                {
                    // lhs is a scalar
                    static_assert(algebra != 'M', "Error in enda::expr: Matrix algebra not supported");
                    return l / r(args...);
                }
                else if constexpr (r_is_scalar)
                    // rhs is a scalar
                    return l(args...) / r;
                else
                {
                    // both are arrays (matrix division is not supported here)
                    static_assert(algebra != 'M', "Error in enda::expr: Matrix algebra not supported");
                    return l(args...) / r(args...);
                }
            }
        }

        template<typename Arg>
        auto operator[](Arg&& arg) const
        {
            static_assert(get_rank<expr> == 1, "Error in enda::expr: Subscript operator only available for expressions of rank 1");
            return operator()(std::forward<Arg>(arg));
        }
    };

    template<Array A>
    expr_unary<'-', A> operator-(A&& a)
    {
        return {std::forward<A>(a)};
    }

    template<Array L, Array R>
    Array auto operator+(L&& l, R&& r)
    {
        static_assert(get_rank<L> == get_rank<R>, "Error in lazy enda::operator+: Rank mismatch");
        return expr<'+', L, R> {std::forward<L>(l), std::forward<R>(r)};
    }

    template<Array A, Scalar S>
    Array auto operator+(A&& a, S&& s)
    {
        return expr<'+', A, std::decay_t<S>> {std::forward<A>(a), s};
    }

    template<Scalar S, Array A>
    Array auto operator+(S&& s, A&& a)
    {
        return expr<'+', std::decay_t<S>, A> {s, std::forward<A>(a)};
    }

    template<Array L, Array R>
    Array auto operator-(L&& l, R&& r)
    {
        static_assert(get_rank<L> == get_rank<R>, "Error in lazy enda::operator-: Rank mismatch");
        return expr<'-', L, R> {std::forward<L>(l), std::forward<R>(r)};
    }

    template<Array A, Scalar S>
    Array auto operator-(A&& a, S&& s)
    {
        return expr<'-', A, std::decay_t<S>> {std::forward<A>(a), s};
    }

    template<Scalar S, Array A>
    Array auto operator-(S&& s, A&& a)
    {
        return expr<'-', std::decay_t<S>, A> {s, std::forward<A>(a)};
    }

    template<Array L, Array R>
    auto operator*(L&& l, R&& r)
    {
        // allowed algebras: A * A or M * M or M * V
        static constexpr char l_algebra = get_algebra<L>;
        static constexpr char r_algebra = get_algebra<R>;
        static_assert(l_algebra != 'V', "Error in enda::operator*: Can not multiply vector by an array or a matrix");

        // two arrays: A * A
        if constexpr (l_algebra == 'A')
        {
            static_assert(r_algebra == 'A', "Error in enda::operator*: Both types need to be arrays");
            static_assert(get_rank<L> == get_rank<R>, "Error in enda::operator*: Rank mismatch");
#ifdef ENDA_ENFORCE_BOUNDCHECK
            if (l.shape() != r.shape())
                ENDA_RUNTIME_ERROR << "Error in enda::operator*: Dimension mismatch: " << l.shape() << " != " << r.shape();
#endif
            return expr<'*', L, R> {std::forward<L>(l), std::forward<R>(r)};
        }

        // two matrices: M * M
        if constexpr (l_algebra == 'M')
        {
            static_assert(l_algebra != 'M', "Not supported in enda::operator*: M * M or M * V.");
        }
    }

    template<Array A, Scalar S>
    Array auto operator*(A&& a, S&& s)
    {
        return expr<'*', A, std::decay_t<S>> {std::forward<A>(a), s};
    }

    template<Scalar S, Array A>
    Array auto operator*(S&& s, A&& a)
    {
        return expr<'*', std::decay_t<S>, A> {s, std::forward<A>(a)};
    }

    template<Array L, Array R>
    Array auto operator/(L&& l, R&& r)
    {
        // allowed algebras: A / A or M / M
        static constexpr char l_algebra = get_algebra<L>;
        static constexpr char r_algebra = get_algebra<R>;
        static_assert(l_algebra != 'V', "Error in enda::operator/: Can not divide a vector by an array or a matrix");

        // two arrays: A / A
        if constexpr (l_algebra == 'A')
        {
            static_assert(r_algebra == 'A', "Error in enda::operator/: Both types need to be arrays");
            static_assert(get_rank<L> == get_rank<R>, "Error in enda::operator/: Rank mismatch");
#ifdef ENDA_ENFORCE_BOUNDCHECK
            if (l.shape() != r.shape())
                ENDA_RUNTIME_ERROR << "Error in enda::operator/: Dimension mismatch: " << l.shape() << " != " << r.shape();
#endif
            return expr<'/', L, R> {std::forward<L>(l), std::forward<R>(r)};
        }

        // two matrices: M / M
        if constexpr (l_algebra == 'M')
        {
            static_assert(l_algebra != 'M', "Not supported in enda::operator*: M / M.");
        }
    }

    template<Array A, Scalar S>
    Array auto operator/(A&& a, S&& s)
    {
        return expr<'/', A, std::decay_t<S>> {std::forward<A>(a), s};
    }

    template<Scalar S, Array A>
    Array auto operator/(S&& s, A&& a)
    {
        static constexpr char algebra = get_algebra<A>;
        if constexpr (algebra == 'M')
            return s * inverse(matrix<get_value_t<A>> {std::forward<A>(a)});
        else
            return expr<'/', std::decay_t<S>, A> {s, std::forward<A>(a)};
    }

} // namespace enda
