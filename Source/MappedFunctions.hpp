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

    inline double abs2(double x) { return x * x; }

    inline double abs2(std::complex<double> z) { return (conj(z) * z).real(); }

    inline bool isnan(std::complex<double> const& z) { return std::isnan(z.real()) or std::isnan(z.imag()); }

    template<typename T>
    T pow(T x, int n) requires(std::is_integral_v<T>)
    {
        T r = 1;
        for (int i = 0; i < n; ++i)
            r *= x;
        return r;
    }

    template<Array A>
    auto pow(A&& a, double p)
    {
        return enda::map([p](auto const& x) {
            using std::pow;
            return pow(x, p);
        })(std::forward<A>(a));
    }

    struct conj_f
    {
        auto operator()(auto const& x) const { return conj(x); };
    };

    template<Array A>
    decltype(auto) conj(A&& a)
    {
        if constexpr (is_complex_v<get_value_t<A>>)
            return enda::map(conj_f {})(std::forward<A>(a));
        else
            return std::forward<A>(a);
    }

    template<Array A>
    auto abs(A&& a)
    {
        return enda::map([](auto const& x) {
            using std::abs;
            return abs(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto imag(A&& a)
    {
        return enda::map([](auto const& x) {
            using std::imag;
            return imag(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto floor(A&& a)
    {
        return enda::map([](auto const& x) {
            using std::floor;
            return floor(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto real(A&& a)
    {
        return enda::map([](auto const& x) { return real(x); })(std::forward<A>(a));
    }

    template<Array A>
    auto abs2(A&& a)
    {
        return enda::map([](auto const& x) { return abs2(x); })(std::forward<A>(a));
    }

    template<Array A>
    auto isnan(A&& a)
    {
        return enda::map([](auto const& x) { return isnan(x); })(std::forward<A>(a));
    }

    template<Array A>
    auto exp(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::exp;
            return exp(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto cos(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::cos;
            return cos(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto sin(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::sin;
            return sin(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto tan(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::tan;
            return tan(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto cosh(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::cosh;
            return cosh(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto sinh(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::sinh;
            return sinh(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto tanh(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::tanh;
            return tanh(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto acos(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::acos;
            return acos(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto asin(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::asin;
            return asin(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto atan(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::atan;
            return atan(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto log(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::log;
            return log(x);
        })(std::forward<A>(a));
    }

    template<Array A>
    auto sqrt(A&& a) requires(get_algebra<A> != 'M')
    {
        return enda::map([](auto const& x) {
            using std::sqrt;
            return sqrt(x);
        })(std::forward<A>(a));
    }

} // namespace enda
