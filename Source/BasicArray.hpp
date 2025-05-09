/**
 * @file BasicArray.hpp
 *
 * @brief Provides the generic class for arrays.
 */

#pragma once

#include <algorithm>
#include <array>
#include <complex>
#include <concepts>
#include <initializer_list>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>

#include "Accessors.hpp"
#include "BasicArrayView.hpp"
#include "BasicFunctions.hpp"
#include "Concepts.hpp"
#include "Exceptions.hpp"
#include "Iterators.hpp"
#include "Layout/ForEach.hpp"
#include "Layout/Permutation.hpp"
#include "Layout/Range.hpp"
#include "LayoutTransforms.hpp"
#include "Macros.hpp"
#include "Mem/AddressSpace.hpp"
#include "Mem/Memcpy.hpp"
#include "Mem/Policies.hpp"
#include "StdUtil/Array.hpp"
#include "Traits.hpp"

#ifdef ENDA_ENFORCE_BOUNDCHECK
    #include <exception>
    #include <iostream>
#endif // ENDA_ENFORCE_BOUNDCHECK

namespace enda
{

    /**
     * @brief A generic multi-dimensional array.
     *
     * @details Together with enda::basic_array_view, this class forms the backbone of the **enda** library. It is templated
     * with the following parameters:
     *
     * - `ValueType`: This is the type of the elements stored in the array. Most of the time, this will be a scalar type
     * like an int, double or std::complex<double>, but it can also be a more complex type like a custom class or a
     * another enda::basic_array.
     * - `Rank`: Integer specifying the number of dimensions of the array. This is a compile-time constant.
     * - `LayoutPolicy`: The layout policy specifies how the array views the memory it uses and how it accesses its
     * elements. It provides a mapping from multi-dimensional to linear indices and vice versa (see @ref layout_pols).
     * - `Algebra`: The algebra specifies how an array behaves when it is used in an expression. Possible values are 'A'
     * (array), 'M' (matrix) and 'V' (vector) (see enda::get_algebra).
     * - `ContainerPolicy`: The container policy specifies how and where the data is stored. It is responsible for
     * allocating/deallocating the memory (see @ref mem_handles).
     *
     * In contrast to views (see enda::basic_array_view), regular arrays own the memory they use for data storage.
     *
     * Arrays and views share a lot of the same operations and functionalities. To turn a view into a regular array, use
     * enda::make_regular.
     *
     * @tparam ValueType Type stored in the array.
     * @tparam Rank Number of dimensions of the array.
     * @tparam LayoutPolicy Policy determining the memory layout.
     * @tparam Algebra Algebra of the array.
     * @tparam ContainerPolicy Policy determining how and where the data is stored.
     */
    template<typename ValueType, int Rank, typename LayoutPolicy, char Algebra, typename ContainerPolicy>
    class basic_array
    {
        // Compile-time checks.
        static_assert(!std::is_const_v<ValueType>, "Error in enda::basic_array: ValueType cannot be const");
        static_assert((Algebra != 'N'), "Internal error in enda::basic_array: Algebra 'N' not supported");
        static_assert((Algebra != 'M') or (Rank == 2), "Internal error in enda::basic_array: Algebra 'M' requires a rank 2 array");
        static_assert((Algebra != 'V') or (Rank == 1), "Internal error in enda::basic_array: Algebra 'V' requires a rank 1 array");

    public:
        // Type of the values in the array (can not be const).
        using value_type = ValueType;

        // Type of the memory layout policy (see @ref layout_pols).
        using layout_policy_t = LayoutPolicy;

        // Type of the memory layout (an enda::idx_map).
        using layout_t = typename LayoutPolicy::template mapping<Rank>;

        // Type of the container policy (see @ref mem_pols).
        using container_policy_t = ContainerPolicy;

        // Type of the memory handle (see @ref mem_handles).
        using storage_t = typename ContainerPolicy::template handle<ValueType>;

        // The associated regular type.
        using regular_type = basic_array;

        // Number of dimensions of the array.
        static constexpr int rank = Rank;

        // Compile-time check.
        static_assert(has_contiguous(layout_t::layout_prop), "Error in enda::basic_array: Memory layout has to be contiguous");

    private:
        // Type of the array itself.
        using self_t = basic_array;

        // Type of the accessor policy for views (no no_alias_accessor).
        using AccessorPolicy = default_accessor;

        // Type of the owning policy for views.
        using OwningPolicy = borrowed<storage_t::address_space>;

        // Constexpr variable that is true if the value_type is const (never for basic_array).
        static constexpr bool is_const = false;

        // Constexpr variable that is true if the array is a view (never for basic_array).
        static constexpr bool is_view = false;

        // Memory layout of the array, i.e. the enda::idx_map.
        layout_t lay;

        // Memory handle of the array.
        storage_t sto;

        // Construct an array with a given shape and initialize the memory with zeros.
        template<std::integral Int = long>
        basic_array(std::array<Int, Rank> const& shape, mem::init_zero_t) : lay {shape}, sto {lay.size(), mem::init_zero}
        {}

    public:
        /**
         * @brief Convert the current array to a view with an 'A' (array) algebra.
         * @return An enda::basic_array_view of the current array.
         */
        auto as_array_view() { return basic_array_view<ValueType, Rank, LayoutPolicy, 'A', AccessorPolicy, OwningPolicy> {*this}; };

        /**
         * @brief Convert the current array to a view with an 'A' (array) algebra.
         * @return An enda::basic_array_view of the current array with const value type.
         */
        auto as_array_view() const { return basic_array_view<const ValueType, Rank, LayoutPolicy, 'A', AccessorPolicy, OwningPolicy> {*this}; };

        // Default constructor constructs an empty array with a default constructed memory handle and layout.
        basic_array() {}; // NOLINT (user-defined constructor to avoid value initialization of the sso buffer)

        // Default move constructor moves the memory handle and layout.
        basic_array(basic_array&&) = default;

        // Default copy constructor copies the memory handle and layout.
        explicit basic_array(basic_array const& a) = default;

        /**
         * @brief Construct an array from another array with a different algebra and/or container policy.
         *
         * @details It takes the memory layout of the other array and copies the data from the memory handle.
         *
         * @tparam A Algebra of the other array.
         * @tparam CP Container policy of the other array.
         * @param a Other array.
         */
        template<char A, typename CP>
        explicit basic_array(basic_array<ValueType, Rank, LayoutPolicy, A, CP> a) noexcept : lay(a.indexmap()), sto(std::move(a.storage()))
        {}

        /**
         * @brief Construct an array with the given dimensions.
         *
         * @details The integer type must be convertible to long and there must be exactly `Rank` arguments. It depends on
         * the value type and the container policy whether the data is initialized with zeros or not.
         *
         * @tparam Ints Integer types.
         * @param is Extent (number of elements) along each dimension.
         */
        template<std::integral... Ints>
        requires(sizeof...(Ints) == Rank) explicit basic_array(Ints... is)
        {
            // setting the layout and storage in the constructor body improves error messages for wrong # of args
            lay = layout_t {std::array {long(is)...}}; // NOLINT (for better error messages)
            sto = storage_t {lay.size()};              // NOLINT (for better error messages)
        }

        /**
         * @brief Construct a 1-dimensional array with the given size and initialize each element to the given scalar value.
         *
         * @tparam Int Integer type.
         * @tparam RHS Type of the scalar value to initialize the array with.
         * @param sz Size of the array.
         * @param val Value to initialize the array with.
         */
        template<std::integral Int, typename RHS>
        explicit basic_array(Int sz, RHS const& val) requires((Rank == 1 and is_scalar_for_v<RHS, basic_array>)) :
            lay(layout_t {std::array {long(sz)}}), sto {lay.size()}
        {
            assign_from_scalar(val);
        }

        /**
         * @brief Construct an array with the given shape.
         *
         * @details It depends on the value type and the container policy whether the data is initialized with zeros or not.
         *
         * @tparam Int Integer type.
         * @param shape Shape of the array.
         */
        template<std::integral Int = long>
        explicit basic_array(std::array<Int, Rank> const& shape) requires(std::is_default_constructible_v<ValueType>) : lay(shape), sto(lay.size())
        {}

        /**
         * @brief Construct an array with the given memory layout.
         *
         * @details It depends on the value type and the container policy whether the data is initialized with zeros or not.
         *
         * @param layout Memory layout.
         */
        explicit basic_array(layout_t const& layout) requires(std::is_default_constructible_v<ValueType>) : lay {layout}, sto {lay.size()} {}

        /**
         * @brief Construct an array with the given memory layout and with an existing memory handle/storage.
         *
         * @details The memory handle is moved and the layout is copied into the array.
         *
         * @param layout Memory layout.
         * @param storage Memory handle/storage.
         */
        explicit basic_array(layout_t const& layout, storage_t&& storage) noexcept : lay {layout}, sto {std::move(storage)} {}

        /**
         * @brief Construct an array from an enda::ArrayOfRank object with the same rank by copying each element.
         *
         * @tparam A enda::ArrayOfRank type.
         * @param a enda::ArrayOfRank object.
         */
        template<ArrayOfRank<Rank> A>
        requires(HasValueTypeConstructibleFrom<A, value_type>) basic_array(A const& a) : lay(a.shape()), sto {lay.size(), mem::do_not_initialize}
        {
            static_assert(std::is_constructible_v<value_type, get_value_t<A>>, "Error in enda::basic_array: Incompatible value types in constructor");
            if constexpr (std::is_trivial_v<ValueType> or is_complex_v<ValueType>)
            {
                // trivial and complex value types can use the optimized assign_from_ndarray
                assign_from_ndarray(a);
            }
            else
            {
                // general value types may not be default constructible -> use placement new
                enda::for_each(lay.lengths(), [&](auto const&... is) { new (sto.data() + lay(is...)) ValueType {a(is...)}; });
            }
        }

        /**
         * @brief Construct an array from an enda::ArrayInitializer object.
         *
         * @details An enda::ArrayInitializer is typically returned by delayed operations (see e.g. enda::mpi_gather).
         * The constructor can then be used to create the resulting array.
         *
         * @tparam Initializer enda::ArrayInitializer type.
         * @param initializer enda::ArrayInitializer object.
         */
        template<ArrayInitializer<basic_array> Initializer> // can not be explicit
        basic_array(Initializer const& initializer) : basic_array {initializer.shape()}
        {
            initializer.invoke(*this);
        }

    private:
        // Get the corresponding shape from an initializer list.
        static std::array<long, 1> shape_from_init_list(std::initializer_list<ValueType> const& l) noexcept { return {long(l.size())}; }

        // Get the corresponding shape from a nested initializer list.
        template<typename L>
        static auto shape_from_init_list(std::initializer_list<L> const& l) noexcept
        {
            const auto [min, max] = std::minmax_element(std::begin(l), std::end(l), [](auto&& x, auto&& y) { return x.size() < y.size(); });
            EXPECTS_WITH_MESSAGE(min->size() == max->size(), "Error in enda::basic_array: Arrays can only be initialized with rectangular initializer lists");
            return stdutil::front_append(shape_from_init_list(*max), long(l.size()));
        }

    public:
        /**
         * @brief Construct a 1-dimensional array from an initializer list.
         * @param l Initializer list.
         */
        basic_array(std::initializer_list<ValueType> const& l) requires(Rank == 1) :
            lay(std::array<long, 1> {long(l.size())}), sto {lay.size(), mem::do_not_initialize}
        {
            long i = 0;
            for (auto const& x : l)
            {
                new (sto.data() + lay(i++)) ValueType {x};
            }
        }

        /**
         * @brief Construct a 2-dimensional array from a double nested initializer list.
         * @param l2 Initializer list.
         */
        basic_array(std::initializer_list<std::initializer_list<ValueType>> const& l2) requires(Rank == 2) :
            lay(shape_from_init_list(l2)), sto {lay.size(), mem::do_not_initialize}
        {
            long i = 0, j = 0;
            for (auto const& l1 : l2)
            {
                for (auto const& x : l1)
                {
                    new (sto.data() + lay(i, j++)) ValueType {x};
                }
                j = 0;
                ++i;
            }
        }

        /**
         * @brief Construct a 3-dimensional array from a triple nested initializer list.
         * @param l3 Initializer list.
         */
        basic_array(std::initializer_list<std::initializer_list<std::initializer_list<ValueType>>> const& l3) requires(Rank == 3) :
            lay(shape_from_init_list(l3)), sto {lay.size(), mem::do_not_initialize}
        {
            long i = 0, j = 0, k = 0;
            for (auto const& l2 : l3)
            {
                for (auto const& l1 : l2)
                {
                    for (auto const& x : l1)
                    {
                        new (sto.data() + lay(i, j, k++)) ValueType {x};
                    }
                    k = 0;
                    ++j;
                }
                j = 0;
                ++i;
            }
        }

        /**
         * @brief Construct a 2-dimensional array from another 2-dimensional array with a different algebra.
         *
         * @details The given array is moved into the constructed array. Note that for enda::stack_array or other array
         * types, this might still involve a copy of the data.
         *
         * @tparam A2 Algebra of the given array.
         * @param a Other array.
         */
        template<char A2>
        explicit basic_array(basic_array<ValueType, 2, LayoutPolicy, A2, ContainerPolicy>&& a) noexcept requires(Rank == 2) :
            basic_array {a.indexmap(), std::move(a).storage()}
        {}

        /**
         * @brief Make a zero-initialized array with the given shape.
         *
         * @tparam Int Integer type.
         * @param shape Shape of the array.
         * @return Zero-initialized array.
         */
        template<std::integral Int = long>
        static basic_array zeros(std::array<Int, Rank> const& shape) requires(std::is_standard_layout_v<ValueType>&& std::is_trivially_copyable_v<ValueType>)
        {
            return basic_array {stdutil::make_std_array<long>(shape), mem::init_zero};
        }

        /**
         * @brief Make a zero-initialized array with the given dimensions.
         *
         * @tparam Ints Integer types.
         * @param is Extent (number of elements) along each dimension.
         * @return Zero-initialized array.
         */
        template<std::integral... Ints>
        static basic_array zeros(Ints... is) requires(sizeof...(Ints) == Rank)
        {
            return zeros(std::array<long, Rank> {is...});
        }

        /**
         * @brief Make a one-initialized array with the given shape.
         *
         * @tparam Int Integer type.
         * @param shape Shape of the array.
         * @return One-initialized array.
         */
        template<std::integral Int = long>
        static basic_array ones(std::array<Int, Rank> const& shape) requires(enda::is_scalar_v<ValueType>)
        {
            auto res = basic_array {stdutil::make_std_array<long>(shape)};
            res()    = ValueType {1};
            return res;
        }

        /**
         * @brief Make a one-initialized array with the given dimensions.
         *
         * @tparam Ints Integer types.
         * @param is Extent (number of elements) along each dimension.
         * @return One-initialized array.
         */
        template<std::integral... Ints>
        static basic_array ones(Ints... is) requires(sizeof...(Ints) == Rank)
        {
            return ones(std::array<long, Rank> {is...});
        }

        /**
         * @brief Make a random-initialized array with the given shape.
         *
         * @details The random values are take from a uniform distribution over [0, 1). For a complex array, both real and
         * imaginary parts are initialized with random values.
         *
         * @tparam Int Integer type.
         * @param shape Shape of the array.
         * @return Random-initialized array.
         */
        template<std::integral Int = long>
        static basic_array rand(std::array<Int, Rank> const& shape) requires(std::is_floating_point_v<ValueType> or enda::is_complex_v<ValueType>)
        {
            using namespace std::complex_literals;
            auto static gen  = std::mt19937(std::random_device {}());
            auto static dist = std::uniform_real_distribution<>(0.0, 1.0);
            auto res         = basic_array {shape};
            if constexpr (enda::is_complex_v<ValueType>)
                for (auto& x : res)
                    x = dist(gen) + 1i * dist(gen);
            else
                for (auto& x : res)
                    x = dist(gen);
            return res;
        }

        /**
         * @brief Make a random-initialized array with the given dimensions.
         *
         * @details The random values are take from a uniform distribution over [0, 1). For a complex array, both real and
         * imaginary parts are initialized with random values.
         *
         * @tparam Ints Integer types.
         * @param is Extent (number of elements) along each dimension.
         * @return Random-initialized array.
         */
        template<std::integral... Ints>
        static basic_array rand(Ints... is) requires(sizeof...(Ints) == Rank)
        {
            return rand(std::array<long, Rank> {is...});
        }

        // Default move assignment moves the memory handle and layout from the right hand side array.
        basic_array& operator=(basic_array&&) = default;

        // Default copy assignment copies the memory handle and layout from the right hand side array.
        basic_array& operator=(basic_array const&) = default;

        /**
         * @brief Assignment operator makes a deep copy of another array with a different algebra and/or container policy.
         *
         * @details A new array object is constructed from the right hand side array and then moved into the current array.
         * This will invalidate all references/views to the existing storage.
         *
         * @tparam A Algebra of the other array.
         * @tparam CP Container policy of the other array.
         * @param rhs Right hand side of the assignment operation.
         */
        template<char A, typename CP>
        basic_array& operator=(basic_array<ValueType, Rank, LayoutPolicy, A, CP> const& rhs)
        {
            *this = basic_array {rhs};
            return *this;
        }

        /**
         * @brief Assignment operator makes a deep copy of an enda::ArrayOfRank object.
         *
         * @details The array is first resized to the shape of the right hand side and the elements are copied. This might
         * invalidate all references/views to the existing storage.
         *
         * @tparam RHS enda::ArrayOfRank type.
         * @param rhs Right hand side of the assignment operation.
         */
        template<ArrayOfRank<Rank> RHS>
        basic_array& operator=(RHS const& rhs)
        {
            resize(rhs.shape());
            assign_from_ndarray(rhs);
            return *this;
        }

        /**
         * @brief Assignment operator assigns a scalar to the array.
         *
         * @details The behavior depends on the algebra of the array:
         * - 'A' (array) and 'V' (vector): The scalar is assigned to all elements of the array.
         * - 'M' (matrix): The scalar is assigned to the diagonal elements of the shorter dimension.
         *
         * @tparam RHS Type of the scalar.
         * @param rhs Right hand side of the assignment operation.
         */
        template<typename RHS>
        basic_array& operator=(RHS const& rhs) noexcept requires(is_scalar_for_v<RHS, basic_array>)
        {
            assign_from_scalar(rhs);
            return *this;
        }

        /**
         * @brief Assignment operator uses an enda::ArrayInitializer to assign to the array.
         *
         * @details The array is resized to the shape of the initializer. This might invalidate all references/views to the
         * existing storage.
         *
         * @tparam Initializer enda::ArrayInitializer type.
         * @param initializer Initializer object.
         */
        template<ArrayInitializer<basic_array> Initializer>
        basic_array& operator=(Initializer const& initializer)
        {
            resize(initializer.shape());
            initializer.invoke(*this);
            return *this;
        }

        /**
         * @brief Resize the array to a new shape.
         *
         * @details Resizing is only performed if the storage is not null and if the new size is different from the previous
         * size. If resizing is performed, the content of the resulting array is undefined since it makes no copy of the
         * previous data and all references/views to the existing storage will be invalidated.
         *
         * @tparam Ints Integer types.
         * @param is New extent (number of elements) along each dimension.
         */
        template<std::integral... Ints>
        void resize(Ints const&... is)
        {
            static_assert(std::is_copy_constructible_v<ValueType>, "Error in enda::basic_array: Resizing requires the value_type to be copy constructible");
            static_assert(sizeof...(is) == Rank, "Error in enda::basic_array: Resizing requires exactly Rank arguments");
            resize(std::array<long, Rank> {long(is)...});
        }

        /**
         * @brief Resize the array to a new shape.
         *
         * @details Resizing is only performed if the storage is not null and if the new size is different from the previous
         * size. If resizing is performed, the content of the resulting array is undefined since it makes no copy of the
         * previous data and all references/views to the existing storage will be invalidated.
         *
         * @param shape New shape of the array.
         */
        void resize(std::array<long, Rank> const& shape)
        {
            lay = layout_t(shape);
            if (sto.is_null() or (sto.size() != lay.size()))
                sto = storage_t {lay.size()};
        }

// include common functionality of arrays and views
#include "ImplBasicArrayViewCommon.hpp"
    };

    // Class template argument deduction guides.
    template<MemoryArray A>
    basic_array(A&& a) -> basic_array<get_value_t<A>,
                                      get_rank<A>,
                                      get_contiguous_layout_policy<get_rank<A>, get_layout_info<A>.stride_order>,
                                      get_algebra<A>,
                                      heap<mem::get_addr_space<A>>>;

    template<Array A>
    basic_array(A&& a) -> basic_array<get_value_t<A>, get_rank<A>, C_layout, get_algebra<A>, heap<>>;

} // namespace enda
