/**
 * @file Product.hpp
 *
 * Cartesian product of input iterables. Equivalent to nested for-loops.
 */

#pragma once

#include <functional>
#include <tuple>
#include <utility>

namespace enda::itertools
{
    // product_iterator is a tuple of regular iterators
    template<typename... Iterators>
    class product_iterator;

    // product_iterator specialization for just one iterator
    template<typename Iterator>
    class product_iterator<Iterator>
    {
    public:
        product_iterator(Iterator it, Iterator last) : _M_it(it), _M_it_last(last) {}

        decltype(auto) operator*() const { return std::make_tuple(*_M_it); }

        product_iterator<Iterator>& operator++()
        {
            ++_M_it;
            return *this;
        }

        bool operator==(const product_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const product_iterator& other) const { return !(*this == other); }

    private:
        Iterator _M_it;
        Iterator _M_it_last;
    };

    // product_iterator general case
    template<typename Iterator, typename... Iterators>
    class product_iterator<Iterator, Iterators...>
    {
    public:
        product_iterator(Iterator                       it,
                         Iterator                       it_last,
                         product_iterator<Iterators...> its,
                         product_iterator<Iterators...> its_first,
                         product_iterator<Iterators...> its_last) : _M_it(it), _M_it_last(it_last), _M_its(its), _M_its_first(its_first), _M_its_last(its_last)
        {}

        decltype(auto) operator*() const { return std::tuple_cat(std::make_tuple(*_M_it), *_M_its); }

        product_iterator& operator++()
        {
            if (++_M_its == _M_its_last && ++_M_it != _M_it_last)
            {
                _M_its = _M_its_first;
            }
            return *this;
        }

        bool operator==(const product_iterator& other) const { return _M_it == other._M_it && _M_its == other._M_its; }

        bool operator!=(const product_iterator& other) const { return !(*this == other); }

    private:
        Iterator                       _M_it;
        Iterator                       _M_it_last;
        product_iterator<Iterators...> _M_its;
        product_iterator<Iterators...> _M_its_first;
        product_iterator<Iterators...> _M_its_last;
    };

    // similar strategy for product_impl
    template<typename... Iterators>
    class product_impl;

    // product_impl specialization for just one iterator
    template<typename Iterator>
    class product_impl<Iterator>
    {
    public:
        product_impl(Iterator first, Iterator last) : _M_it_first(first), _M_it_last(last) {}

        auto begin() const { return product_iterator<Iterator>(_M_it_first, _M_it_last); }

        auto end() const { return product_iterator<Iterator>(_M_it_last, _M_it_last); }

    private:
        Iterator _M_it_first;
        Iterator _M_it_last;
    };

    // product_impl general case
    template<typename Iterator, typename... Iterators>
    class product_impl<Iterator, Iterators...>
    {
    public:
        template<typename... ArgsT>
        product_impl(Iterator first, Iterator last, ArgsT... rest) : _M_it_first(first), _M_it_last(last), _M_sub_product(rest...)
        {}

        auto begin() const
        {
            return product_iterator<Iterator, Iterators...>(_M_it_first, _M_it_last, _M_sub_product.begin(), _M_sub_product.begin(), _M_sub_product.end());
        }

        auto end() const
        {
            return product_iterator<Iterator, Iterators...>(_M_it_last, _M_it_last, _M_sub_product.end(), _M_sub_product.begin(), _M_sub_product.end());
        }

    private:
        Iterator                   _M_it_first;
        Iterator                   _M_it_last;
        product_impl<Iterators...> _M_sub_product;
    };

    // finally, the product() function
    template<typename... Iterables>
    auto product(Iterables&&... iterables)
    {
        return std::apply([&iterables...](auto&&... args) { return product_impl<decltype(iterables.begin())...>(std::forward<decltype(args)>(args)...); },
                          std::tuple_cat(std::make_tuple(iterables.begin(), iterables.end())...));
    }

} // namespace enda::itertools
