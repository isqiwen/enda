/**
 * @file MappedFunctions.hpp
 *
 * @brief Provides some custom implementations of standard mathematical functions used for lazy, coefficient-wise array
 * operations.
 */

#pragma once

#include <cmath>
#include <complex>
#include <type_traits>
#include <utility>

#include "Concepts.hpp"
#include "Map.hpp"
#include "Traits.hpp"

namespace enda
{

    /**
     * @addtogroup av_math
     * @{
     */

    /**
     * @brief Get the real part of a scalar.
     *
     * @tparam T Scalar type.
     * @param t Scalar value.
     * @return Real part of the scalar.
     */
    template<typename T>
    auto real(T t) requires(enda::is_scalar_v<T>)
    {
        if constexpr (is_complex_v<T>)
        {
            return std::real(t);
        }
        else
        {
            return t;
        }
    }

    /**
     * @brief Get the complex conjugate of a scalar.
     *
     * @tparam T Scalar type.
     * @param t Scalar value.
     * @return The given scalar if it is not complex, otherwise its complex conjugate.
     */
    template<typename T>
    auto conj(T t) requires(enda::is_scalar_v<T>)
    {
        if constexpr (is_complex_v<T>)
        {
            return std::conj(t);
        }
        else
        {
            return t;
        }
    }

    /**
     * @brief Get the squared absolute value of a double.
     *
     * @param x Double value.
     * @return Squared absolute value of the given double.
     */
    inline double abs2(double x) { return x * x; }

    /**
     * @brief Get the squared absolute value of a std::complex<double>.
     *
     * @param z std::complex<double> value.
     * @return Squared absolute value of the given complex number.
     */
    inline double abs2(std::complex<double> z) { return (conj(z) * z).real(); }

    /**
     * @brief Check if a std::complex<double> is NaN.
     *
     * @param z std::complex<double> value.
     * @return True if either the real or imaginary part of the given complex number is `NaN`, false otherwise.
     */
    inline bool isnan(std::complex<double> const& z) { return std::isnan(z.real()) or std::isnan(z.imag()); }

    /**
     * @brief Calculate the integer power of an integer.
     *
     * @tparam T Integer type.
     * @param x Base value.
     * @param n Exponent value.
     * @return The result of the base raised to the power of the exponent.
     */
    template<typename T>
    T pow(T x, int n) requires(std::is_integral_v<T>)
    {
        T r = 1;
        for (int i = 0; i < n; ++i)
            r *= x;
        return r;
    }

    /**
     * @brief Lazy, coefficient-wise power function for enda::Array types.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @param p Exponent value.
     * @return A lazy enda::expr_call object.
     */
    template<Array A>
    auto pow(A&& a, double p)
    {
        return enda::map([p](auto const& x) {
            using std::pow;
            return pow(x, p);
        })(std::forward<A>(a));
    }

    /// Wrapper for enda::conj.
    struct conj_f
    {
        /// Function call operator that forwards the call to enda::conj.
        auto operator()(auto const& x) const { return conj(x); };
    };

    /**
     * @brief Lazy, coefficient-wise complex conjugate function for enda::Array types.
     *
     * @tparam A enda::Array type.
     * @param a enda::Array object.
     * @return A lazy enda::expr_call object if the array is complex valued, otherwise the array itself.
     */
    template<Array A>
    decltype(auto) conj(A&& a)
    {
        if constexpr (is_complex_v<get_value_t<A>>)
            return enda::map(conj_f {})(std::forward<A>(a));
        else
            return std::forward<A>(a);
    }

    /**
     * @brief Lazy, coefficient-wise abs function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto abs(A&& a)
    {
        return nda::map([](auto const& x) {
            using std::abs;
            return abs(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise imag function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto imag(A&& a)
    {
        return nda::map([](auto const& x) {
            using std::imag;
            return imag(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise floor function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto floor(A&& a)
    {
        return nda::map([](auto const& x) {
            using std::floor;
            return floor(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise real function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto real(A&& a)
    {
        return nda::map([](auto const& x) { return real(x); })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise abs2 function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto abs2(A&& a)
    {
        return nda::map([](auto const& x) { return abs2(x); })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise isnan function for nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto isnan(A&& a)
    {
        return nda::map([](auto const& x) { return isnan(x); })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise exp function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto exp(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::exp;
            return exp(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise cos function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto cos(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::cos;
            return cos(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise sin function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto sin(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::sin;
            return sin(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise tan function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto tan(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::tan;
            return tan(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise cosh function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto cosh(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::cosh;
            return cosh(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise sinh function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto sinh(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::sinh;
            return sinh(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise tanh function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto tanh(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::tanh;
            return tanh(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise acos function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto acos(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::acos;
            return acos(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise asin function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto asin(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::asin;
            return asin(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise atan function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto atan(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::atan;
            return atan(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise log function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto log(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::log;
            return log(x);
        })(std::forward<A>(a));
    }

    /**
     * @brief Lazy, coefficient-wise sqrt function for non-matrix nda::Array types.
     *
     * @tparam A nda::Array type.
     * @param a nda::Array object.
     * @return A lazy nda::expr_call object.
     */
    template<Array A>
    auto sqrt(A&& a) requires(get_algebra<A> != 'M')
    {
        return nda::map([](auto const& x) {
            using std::sqrt;
            return sqrt(x);
        })(std::forward<A>(a));
    }

    /** @} */

} // namespace enda
