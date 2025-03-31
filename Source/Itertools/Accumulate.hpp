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
#include <memory>
#include <vector>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Iterator, typename S, typename Fn>
    class accumulate_iterator
    {
    public:
        accumulate_iterator(Iterator first, Iterator last, S init, Fn fn) :
            _M_it(first), _M_last(last), _M_s(init), _M_fn(fn), _M_first_yielded(false)
        {}

        decltype(auto) operator*() const { return _M_s; }

        accumulate_iterator& operator++()
        {
            if (!_M_first_yielded)
            {
                _M_first_yielded = true;
            }

            if (_M_it != _M_last)
            {
                ++_M_it;
                if (_M_it != _M_last)
                {
                    _M_s = _M_fn(std::move(_M_s), *_M_it);
                }
            }
            return *this;
        }

        bool operator==(const accumulate_iterator& other) const { return _M_it == other._M_it && _M_first_yielded == other._M_first_yielded; }

        bool operator!=(const accumulate_iterator& other) const { return !(*this == other); }

        static accumulate_iterator end_iterator(Iterator last, S init, Fn fn)
        {
            accumulate_iterator it(last, last, init, fn);
            it._M_first_yielded = true;
            return it;
        }

    private:
        Iterator _M_it;
        Iterator _M_last;
        S        _M_s;
        Fn       _M_fn;
        bool     _M_first_yielded;
        bool     _M_first_element = false;
    };

    template<class TL, class TR, class TResult>
    struct plus
    {
        TResult operator()(const TL& x, const TR& y) const { return x + y; }
    };

    template<typename Iterator, typename S, typename Fn>
    auto accumulate(Iterator first, Iterator last, S init, Fn fn)
    {
        accumulate_iterator<Iterator, S, Fn> a_it_first(first, last, (first == last) ? init : fn(init, *first), fn);
        accumulate_iterator<Iterator, S, Fn> a_it_last = accumulate_iterator<Iterator, S, Fn>::end_iterator(last, init, fn);
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
