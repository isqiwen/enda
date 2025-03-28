/**
 * @file Count.hpp
 *
 * An infinite counter.
 */

#pragma once

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename T, typename S>
    class count_iterator
    {
    public:
        count_iterator(T start, S step) : _M_start(start), _M_step(step) {}

        decltype(auto) operator*() const { return _M_start; }

        count_iterator& operator++()
        {
            _M_start += _M_step;
            return *this;
        }

        bool operator==(const count_iterator& other) const { return false; }

        bool operator!=(const count_iterator& other) const { return !(*this == other); }

    private:
        T _M_start;
        S _M_step;
    };

    template<typename T, typename S>
    auto count(T start, S step)
    {
        using c_it_t = count_iterator<T, S>;
        return range_view<c_it_t>(c_it_t(start, step), c_it_t(start, step));
    }

} // namespace enda::itertools
