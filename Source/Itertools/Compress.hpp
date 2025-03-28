/**
 * @file Compress.hpp
 *
 * Make an iterator that filters elements from data returning only those that have a
 * corresponding element in selectors that evaluates to True.
 * Stops when either the data or selectors iterables has been exhausted.
 */

#pragma once

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename DIterator, typename SIterator>
    class compress_iterator
    {
    public:
        compress_iterator(DIterator d_it, DIterator d_it_last, SIterator s_it, SIterator s_it_last) :
            _M_d_it(d_it), _M_d_it_last(d_it_last), _M_s_it(s_it), _M_s_it_last(s_it_last)
        {
            next_selected();
        }

        void next_selected()
        {
            for (; _M_s_it != _M_s_it_last && _M_d_it != _M_d_it_last && !(*_M_s_it); ++_M_s_it, ++_M_d_it)
                ;
        }

        decltype(auto) operator*() const { return *_M_d_it; }

        compress_iterator& operator++()
        {
            ++_M_d_it;
            ++_M_s_it;
            next_selected();
            return *this;
        }

        bool operator==(const compress_iterator& other) const { return _M_d_it == other._M_d_it || _M_s_it == other._M_s_it; }

        bool operator!=(const compress_iterator& other) const { return !(*this == other); }

    private:
        DIterator _M_d_it;
        DIterator _M_d_it_last;
        SIterator _M_s_it;
        SIterator _M_s_it_last;
    };

    template<typename DIterator, typename SIterator>
    auto compress(DIterator data_first, DIterator data_last, SIterator selector_first, SIterator selector_last)
    {
        using it_t = compress_iterator<DIterator, SIterator>;
        it_t it_first(data_first, data_last, selector_first, selector_last);
        it_t it_last(data_last, data_last, selector_last, selector_last);
        return range_view<it_t>(it_first, it_last);
    }

    template<typename DIterable, typename SIterable>
    auto compress(DIterable&& data, SIterable&& selectors)
    {
        return compress(data.begin(), data.end(), selectors.begin(), selectors.end());
    }

} // namespace itertools
