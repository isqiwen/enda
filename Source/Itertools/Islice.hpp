/**
 * @file Islice.hpp
 *
 * Return an iterator whose next() method returns selected values from an iterable.
 *
 * If start is specified, will skip all preceding elements; otherwise, start defaults to zero.
 *
 * step defaults to one. If specified as another value, step determines how many values are
 * skipped between successive calls.
 */

#pragma once

#include <stdexcept>

#include "Itertools/RangeView.hpp"

namespace enda::itertools
{
    template<typename Iterator, typename Index, typename Step>
    class islice_iterator
    {
    public:
        islice_iterator(Iterator first, Iterator last, Index idx, Index stop, Step step) :
            _M_it(first), _M_it_last(last), _M_idx(idx), _M_idx_stop(stop), _M_idx_step(step)
        {}

        decltype(auto) operator*() const { return *_M_it; }

        islice_iterator& operator++()
        {
            for (Step step = 0; step != _M_idx_step; ++step)
            {
                if (_M_it == _M_it_last)
                {
                    break;
                }
                if (_M_idx == _M_idx_stop)
                {
                    break;
                }
                ++_M_it;
                ++_M_idx;
            }
            return *this;
        }

        bool operator==(const islice_iterator& other) const { return _M_it == other._M_it; }

        bool operator!=(const islice_iterator& other) const { return !(*this == other); }

    private:
        Iterator _M_it;
        Iterator _M_it_last;
        Index    _M_idx;
        Index    _M_idx_stop;
        Step     _M_idx_step;
    };

    template<typename Iterator, typename Index, typename Step>
    auto islice(Iterator first, Iterator last, Index start, Index stop, Step step)
    {
        using it_t = islice_iterator<Iterator, Index, Step>;
        it_t it_last(last, last, start, stop, step);
        if (0 < step)
        {
            if (start < stop)
            {
                for (Index i = 0; i < start && first != last; ++i, ++first)
                    ;
                return range_view<it_t>(it_t(first, last, start, stop, step), it_last);
            }
            else
            {
                // empty range
                return range_view<it_t>(it_last, it_last);
            }
        }
        else
        {
            // negative step not supported yet
            throw std::runtime_error("islice() step is non-positive");
        }
    }

    template<typename Iterable, typename Index, typename Step>
    auto islice(Iterable&& iterable, Index start, Index stop, Step step)
    {
        return islice(iterable.begin(), iterable.end(), start, stop, step);
    }

} // namespace enda::itertools
