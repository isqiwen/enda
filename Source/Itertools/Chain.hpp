/**
 * @file Chain.hpp
 *
 * Return a chain object whose .__next__() method returns elements from the
 * first iterable until it is exhausted, then elements from the next
 * iterable, until all of the iterables are exhausted.
 */

#pragma once

#include <tuple>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Value, typename... Iterator>
    class chain_iterator;

    // base case
    template<typename Value, typename Iterator>
    class chain_iterator<Value, Iterator>
    {
    public:
        chain_iterator(Iterator it, Iterator last) : _M_it(it), _M_it_last(last) {}

        Value& operator*() const { return *_M_it; }

        chain_iterator operator++()
        {
            ++_M_it;
            return *this;
        }

        bool operator==(const chain_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const chain_iterator& other) const { return !(*this == other); }

    private:
        Iterator _M_it;
        Iterator _M_it_last;
    };

    // general case
    template<typename Value, typename Iterator, typename... Iterators>
    class chain_iterator<Value, Iterator, Iterators...>
    {
    public:
        template<typename... Args>
        chain_iterator(Iterator it, Iterator last, Args... rest) : _M_it(it), _M_it_last(last), _M_sub_chain_it(rest...)
        {}

        Value& operator*() const { return _M_it != _M_it_last ? *_M_it : *_M_sub_chain_it; }

        chain_iterator operator++()
        {
            if (_M_it != _M_it_last)
            {
                ++_M_it;
            }
            else
            {
                ++_M_sub_chain_it;
            }
            return *this;
        }

        bool operator==(const chain_iterator& other) const { return _M_it == other._M_it && _M_sub_chain_it == other._M_sub_chain_it; }

        bool operator!=(const chain_iterator& other) const { return !(*this == other); }

    private:
        Iterator                            _M_it;
        Iterator                            _M_it_last;
        chain_iterator<Value, Iterators...> _M_sub_chain_it;
    };

    template<typename Iterable, typename... Iterables>
    auto chain(Iterable&& iterable, Iterables&&... iterables)
    {
        using value_type          = decltype(*iterable.begin());
        using chain_iterator_type = chain_iterator<value_type, decltype(iterable.begin()), decltype(iterables.begin())...>;

        auto first = std::apply([&iterable, &iterables...](auto&&... args) { return chain_iterator_type(std::forward<decltype(args)>(args)...); },
                                std::tuple_cat(std::make_tuple(iterable.begin(), iterable.end()), std::make_tuple(iterables.begin(), iterables.end())...));

        auto last = std::apply([&iterable, &iterables...](auto&&... args) { return chain_iterator_type(std::forward<decltype(args)>(args)...); },
                               std::tuple_cat(std::make_tuple(iterable.end(), iterable.end()), std::make_tuple(iterables.end(), iterables.end())...));

        return range_view<chain_iterator_type>(first, last);
    }

} // namespace enda::itertools
