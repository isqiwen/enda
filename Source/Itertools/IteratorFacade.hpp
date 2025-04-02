/**
 * @file IteratorFacade.hpp
 *
 * @brief Provides a CRTP base class for various iterator types in itertools.
 */

#include <cstddef>
#include <iterator>

namespace enda::itertools
{

    /// @cond
    // Forward declaration.
    template<typename Iter, typename Value, typename Tag = std::forward_iterator_tag, typename Reference = Value&, typename Difference = std::ptrdiff_t>
    struct iterator_facade;
    /// @endcond

    /**
     * @ingroup utilities
     * @brief CRTP base class for various iterator types in itertools.
     *
     * @details All iterator types defined in itertools are derived from this class. It uses the
     * <a href="https://en.cppreference.com/w/cpp/iterator/forward_iterator">forward iterator</a>
     * category. Derived classes are required to implement the following member functions:
     *
     * @code{.cpp}
     * // used by operator++() and operator++(int)
     * void increment();
     *
     * // used by operator*() and operator->()
     * value_type [const] [&] dereference() [const];
     * @endcode
     *
     * The `[..]` are optional and depend on the actual iterator type.
     *
     * @tparam Iter Derived iterator type.
     * @tparam Value Value type of the iterator.
     * @tparam Reference Reference type of the iterator.
     * @tparam Difference Difference type of the iterator.
     */
    template<typename Iter, typename Value, typename Reference, typename Difference>
    struct iterator_facade<Iter, Value, std::forward_iterator_tag, Reference, Difference>
    {
    private:
        // Get a reference to the derived iterator.
        [[nodiscard]] Iter& self() { return static_cast<Iter&>(*this); }

        // Get a const reference to the derived iterator.
        [[nodiscard]] Iter const& self() const { return static_cast<const Iter&>(*this); }

    public:
        /// Value type of the derived iterator.
        using value_type = Value;

        /// Reference type of the derived iterator.
        using reference = Reference;

        /// Pointer type of the derived iterator.
        using pointer = Value*;

        /// Difference type of the derived iterator.
        using difference_type = Difference;

        /// Iterator category of the derived iterator.
        using iterator_category = std::forward_iterator_tag;

        /**
         * @brief Pre-increment operator.
         * @return Reference to the derived iterator after calling the derived iterator's `increment()` function.
         */
        Iter& operator++()
        {
            self().increment();
            return self();
        }

        /**
         * @brief Post-increment operator.
         * @return Copy of the derived iterator before calling the derived iterator's `increment()` function.
         */
        Iter operator++(int)
        {
            Iter c = self();
            self().increment();
            return c;
        }

        /**
         * @brief Dereference operator.
         * @return Result of the derived iterator's `dereference()` function.
         */
        [[nodiscard]] decltype(auto) operator*() const { return self().dereference(); }

        /**
         * @brief Member access operator.
         * @return Result of the derived iterator's `dereference()` function.
         */
        [[nodiscard]] decltype(auto) operator->() const { return operator*(); }
    };

} // namespace enda::itertools
