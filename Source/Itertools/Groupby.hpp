/**
 * @file Groupby.hpp
 *
 * Make an iterator that returns consecutive keys and groups from the iterable.
 *
 * groupby(["AAABBBBCCD"]) -> ("A", "AAA"), ("B", "BBBB"), ("C", "CC"), ("D", "D")
 *
 * The key is a function computing a key value for each element.
 * If not specified or is None, key defaults to an identity function and returns the element unchanged.
 * Generally, the iterable needs to already be sorted on the same key function.
 *
 * The operation of groupby() is similar to the uniq filter in Unix. It generates a break or new group
 * every time the value of the key function changes (which is why it is usually necessary
 * to have sorted the data using the same key function).
 * That behavior differs from SQL’s GROUP BY which aggregates common elements regardless of their input order.
 *
 * The returned group is itself an iterator that shares the underlying iterable with groupby().
 * Because the source is shared, when the groupby() object is advanced, the previous group is no longer visible.
 */

#pragma once

#include <tuple>
#include <type_traits>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Iterator, typename Fn>
    class groupby_iterator
    {
        // return type of Fn
        using key_t = typename std::remove_reference<typename std::invoke_result<Fn, decltype(*Iterator())>::type>::type;

    public:
        groupby_iterator(Iterator first, Iterator last, Fn key_fn) : _M_it(first), _M_it_last(last), _M_group_it_first(first), _M_key_fn(key_fn)
        {
            // compute initial key and group
            if (_M_it != _M_it_last)
            {
                _M_key = _M_key_fn(*_M_it);
                ++_M_it; // so don't repeat computing key on the same element (first)
                next_group();
            }
        }

        /// \brief Advance _M_it until _M_key_fn(*_M_it) yields a different key, or _M_it reaches last.
        void next_group()
        {
            for (; _M_it != _M_it_last && (_M_next_key = _M_key_fn(*_M_it)) == _M_key; ++_M_it)
                ;
        }

        decltype(auto) operator*() const { return std::make_tuple(_M_key, range_view(_M_group_it_first, _M_it)); }

        groupby_iterator& operator++()
        {
            if (_M_it == _M_it_last)
            {
                _M_group_it_first = _M_it;
                return *this;
            }
            _M_group_it_first = _M_it;
            _M_key            = _M_next_key;
            ++_M_it; // so don't repeat computing key on the same element
            next_group();
            return *this;
        }

        bool operator==(const groupby_iterator& other) const { return _M_group_it_first == other._M_group_it_first; }

        bool operator!=(const groupby_iterator& other) const { return !(*this == other); }

    private:
        Iterator _M_it; ///< _M_group_it_last
        Iterator _M_it_last;
        Iterator _M_group_it_first;
        Fn       _M_key_fn;
        key_t    _M_key;
        key_t    _M_next_key; ///< should always equal to _M_key_fn(*_M_it)
    };

    template<typename T>
    struct identity_fn
    {
        const T& operator()(const T& arg) const { return arg; }
    };

    template<typename Iterator, typename Fn>
    auto groupby(Iterator first, Iterator last, Fn fn)
    {
        using it_t = groupby_iterator<Iterator, Fn>;
        return range_view<it_t>(it_t(first, last, fn), it_t(last, last, fn));
    }

    template<typename Iterator>
    auto groupby(Iterator first, Iterator last)
    {
        using Fn = identity_fn<decltype(*first)>;
        return groupby(first, last, Fn());
    }

    template<typename Iterable, typename Fn>
    auto groupby(Iterable&& iterable, Fn fn)
    {
        return groupby(iterable.begin(), iterable.end(), fn);
    }

    template<typename Iterable>
    auto groupby(Iterable&& iterable)
    {
        return groupby(iterable.begin(), iterable.end());
    }

} // namespace enda::itertools
