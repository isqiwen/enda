/**
 * @file Accumulate.hpp
 *
 * Make an iterator that returns accumulated sums, or accumulated results of
 * other binary functions (specified via the optional func argument).
 *
 * If func is supplied, it should be a function of two arguments.
 * Elements of the input iterable may be any type that can be accepted as arguments to func.
 */

#pragma once

#include <functional>
#include <utility>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Iterator, typename S, typename Fn>
    class accumulate_iterator
    {
    public:
        accumulate_iterator(Iterator it, S init, Fn fn) : _M_it(it), _M_s(init), _M_fn(fn) {}

        decltype(auto) operator*() const { return _M_s; }

        accumulate_iterator& operator++()
        {
            _M_s = _M_fn(std::move(_M_s), *(++_M_it));
            return *this;
        }

        bool operator==(const accumulate_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const accumulate_iterator& other) const { return !(*this == other); }

    private:
        Iterator _M_it;
        S        _M_s;
        Fn       _M_fn;
    };

    template<class TL, class TR, class TResult>
    struct plus
    {
        TResult operator()(const TL& x, const TR& y) const { return x + y; }
    };

    template<typename Iterator, typename S, typename Fn>
    auto accumulate(Iterator first, Iterator last, S init, Fn fn)
    {
        accumulate_iterator<Iterator, S, Fn> a_it_first(first, (first == last) ? init : fn(init, *first), fn);
        accumulate_iterator<Iterator, S, Fn> a_it_last(last, init, fn);
        return range_view(a_it_first, a_it_last);
    }

    template<typename Iterable, typename S, typename Fn>
    auto accumulate(const Iterable& iterable, S init, Fn fn)
    {
        return accumulate(iterable.begin(), iterable.end(), init, fn);
    }

    template<typename Iterator, typename S>
    auto accumulate(Iterator first, Iterator last, S init)
    {
        return accumulate(first, last, init, plus<S, decltype(*first), S>());
    }

    template<typename Iterable, typename S>
    auto accumulate(const Iterable& iterable, S init)
    {
        return accumulate(iterable.begin(), iterable.end(), init, plus<S, decltype(*iterable.begin()), S>());
    }

} // namespace enda::itertools
