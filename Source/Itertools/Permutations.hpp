/**
 * @file Permutations.hpp
 *
 * Return successive r length permutations of elements in the iterable.
 *
 * If r is not specified or is None, then r defaults to the length of the iterable
 * and all possible full-length permutations are generated.
 *
 * Permutations are emitted in lexicographic sort order. So, if the input iterable is sorted,
 * the permutation tuples will be produced in sorted order.
 *
 * Elements are treated as unique based on their position, not on their value.
 * So if the input elements are unique, there will be no repeat values in each permutation.
 */

#pragma once

#include <functional>
#include <tuple>
#include <utility>

namespace enda::itertools
{
    template<typename... Iterators>
    class permutations_iterator;

    template<typename Iterator>
    class permutations_iterator<Iterator>
    {
    public:
        permutations_iterator(Iterator it, Iterator last) : _M_it(it), _M_it_last(last) {}

        decltype(auto) operator*() const { return std::make_tuple(*_M_it); }

        permutations_iterator<Iterator>& operator++()
        {
            ++_M_it;
            return *this;
        }

        bool repeated(const Iterator& it) const { return _M_it == it; }

        bool operator==(const permutations_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const permutations_iterator& other) const { return !(*this == other); }

    protected:
        Iterator _M_it;
        Iterator _M_it_last;
    };

    template<typename Iterator, typename... Iterators>
    class permutations_iterator<Iterator, Iterators...>
    {
    public:
        permutations_iterator(Iterator                            it,
                              Iterator                            it_last,
                              permutations_iterator<Iterators...> its,
                              permutations_iterator<Iterators...> its_first,
                              permutations_iterator<Iterators...> its_last) :
            _M_it(it), _M_it_last(it_last), _M_its(its), _M_its_first(its_first), _M_its_last(its_last)
        {}

        decltype(auto) operator*() const { return std::tuple_cat(std::make_tuple(*_M_it), *_M_its); }

        permutations_iterator& operator++()
        {
            do
            {
                if (++_M_its == _M_its_last)
                {
                    if (++_M_it == _M_it_last)
                    {
                        break;
                    }
                    _M_its = _M_its_first;
                }
            } while (_M_its.repeated(_M_it));

            return *this;
        }

        bool repeated(const Iterator& it) const { return _M_it == it || _M_its.repeated(it); }

        bool operator==(const permutations_iterator& other) const { return _M_it == other._M_it && _M_its == other._M_its; }

        bool operator!=(const permutations_iterator& other) const { return !(*this == other); }

    protected:
        Iterator                            _M_it;
        Iterator                            _M_it_last;
        permutations_iterator<Iterators...> _M_its;
        permutations_iterator<Iterators...> _M_its_first;
        permutations_iterator<Iterators...> _M_its_last;
    };

    template<typename... Iterables>
    class permutations_impl;

    template<typename Iterator>
    class permutations_impl<Iterator>
    {
    public:
        permutations_impl(Iterator first, Iterator last) : _M_it_first(first), _M_it_last(last) {}

        auto begin() const { return permutations_iterator<Iterator>(_M_it_first, _M_it_last); }

        auto end() const { return permutations_iterator<Iterator>(_M_it_last, _M_it_last); }

    protected:
        Iterator _M_it_first;
        Iterator _M_it_last;
    };

    template<typename Iterator, typename... Iterators>
    class permutations_impl<Iterator, Iterators...>
    {
    public:
        template<typename... ArgsT>
        permutations_impl(Iterator first, Iterator last, ArgsT... rest) : _M_it_first(first), _M_it_last(last), _M_sub_product(rest...)
        {}

        auto begin() const
        {
            return permutations_iterator<Iterator, Iterators...>(_M_it_first, _M_it_last, _M_sub_product.begin(), _M_sub_product.begin(), _M_sub_product.end());
        }

        auto end() const
        {
            return permutations_iterator<Iterator, Iterators...>(_M_it_last, _M_it_last, _M_sub_product.end(), _M_sub_product.begin(), _M_sub_product.end());
        }

    protected:
        Iterator                        _M_it_first;
        Iterator                        _M_it_last;
        permutations_impl<Iterators...> _M_sub_product;
    };

    template<typename... Iterables>
    decltype(auto) __permutations(Iterables&&... iterables)
    {
        return std::apply([&iterables...](auto&&... args) { return permutations_impl<decltype(iterables.begin())...>(std::forward<decltype(args)>(args)...); },
                          std::tuple_cat(std::make_tuple(iterables.begin(), iterables.end())...));
    }

    template<std::size_t N, typename T>
    decltype(auto) clone_args(T&& arg)
    {
        if constexpr (N == 0)
        {
            return std::tuple<>();
        }
        else
        {
            return std::tuple_cat(std::tuple<T&&>(std::forward<T>(arg)), clone_args<N - 1>(std::forward<T>(arg)));
        }
    }

    template<std::size_t N, typename Iterable>
    auto permutations(Iterable&& iterable)
    {
        // call __permutations(iterable, iterable, ...); // N * iterables
        static_assert(N > 0);
        return std::apply([](auto&&... args) { return __permutations(std::forward<decltype(args)>(args)...); },
                          clone_args<N>(std::forward<Iterable>(iterable)));
    }

} // namespace enda::itertools
