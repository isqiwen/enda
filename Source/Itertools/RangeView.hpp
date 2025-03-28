/**
 * @file RangeView.hpp
 *
 * Wrap iterators [first, last) into an object that has begin() and end().
 */

#pragma once

namespace enda::itertools
{
    template<typename Iterator>
    class range_view
    {
    public:
        range_view(Iterator first, Iterator last) : _M_first(first), _M_last(last) {}

        Iterator begin() const { return _M_first; }

        Iterator end() const { return _M_last; }

    private:
        Iterator _M_first;
        Iterator _M_last;
    };

} // namespace enda::itertools
