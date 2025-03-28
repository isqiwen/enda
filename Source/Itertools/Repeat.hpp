/**
 * @file Repeat.hpp
 *
 * Make an iterator that returns object over and over again.
 * Runs indefinitely unless the times argument is specified.
 */

#pragma once

#include <utility>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Value>
    class repeat_inf_iterator
    {
    public:
        repeat_inf_iterator(Value&& value) : _M_value(std::forward<Value>(value)) {}

        Value& operator*() const { return _M_value; }

        repeat_inf_iterator& operator++() { return *this; }

        bool operator==(const repeat_inf_iterator& other) const { return false; }

        bool operator!=(const repeat_inf_iterator& other) const { return true; }

    private:
        Value _M_value;
    };

    template<typename Value, typename N>
    class repeat_iterator
    {
    public:
        repeat_iterator(Value&& value, N times) : _M_value(std::forward<Value>(value)), _M_times(times) {}

        Value& operator*() const { return _M_value; }

        repeat_iterator& operator++()
        {
            ++_M_times;
            return *this;
        }

        bool operator==(const repeat_iterator& other) const { return _M_times == other._M_times; }

        bool operator!=(const repeat_iterator& other) const { return !(*this == other); }

    private:
        Value _M_value;
        N     _M_times;
    };

    template<typename Value>
    auto repeat(Value&& value)
    {
        using it_t = repeat_inf_iterator<Value>;
        it_t it_first(std::forward<Value>(value));
        it_t it_last(std::forward<Value>(value));
        return range_view<it_t>(it_first, it_last);
    }

    template<typename Value, typename N>
    auto repeat(Value&& value, N times)
    {
        using it_t = repeat_iterator<Value, N>;
        it_t it_first(std::forward<Value>(value), 0);
        it_t it_last(std::forward<Value>(value), times);
        return range_view<it_t>(it_first, it_last);
    }

} // namespace enda::itertools
