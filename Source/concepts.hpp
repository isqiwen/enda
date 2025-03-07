#pragma once

#include <array>
#include <concepts>
#include <type_traits>
#include <utility

#include "StdUtil/Concepts.hpp"
#include "Traits.hpp"

namespace enda
{
    namespace detail
    {

        // Helper function declaration to check if A can be called with R long arguments.
        template<auto... Is, typename A>
        auto call_on_R_longs(std::index_sequence<Is...>, A const& a) -> decltype(a((long {Is})...)); // no impl needed

    } // namespace detail

    /**
     * @brief Check if a given type can be called with a certain number of long arguments.
     *
     * @details An example of a type satisfying this concept is e.g. enda::basic_array or enda::basic_array_view. More
     * generally, every type modelling the enda::Array concept has to be enda::CallableWithLongs as well.
     *
     * @tparam A Type to check.
     * @tparam R Number of long arguments.
     */
    template<typename A, int R>
    concept CallableWithLongs = requires(A const& a)
    {
        {detail::call_on_R_longs(std::make_index_sequence<R> {}, a)};
    };

    namespace detail
    {
        // Check if T is of type std::array<long, R> for arbitrary array sizes R.
        template<typename T>
        constexpr bool is_std_array_of_long_v = false;

        // Specialization of enda::detail::is_std_array_of_long_v for cvref types.
        template<typename T>
        requires(!std::is_same_v<T, std::remove_cvref_t<T>>) constexpr bool is_std_array_of_long_v<T> = is_std_array_of_long_v<std::remove_cvref_t<T>>;

        // Specialization of enda::detail::is_std_array_of_long_v for std::array<long, R>.
        template<auto R>
        constexpr bool is_std_array_of_long_v<std::array<long, R>> = true;

    } // namespace detail

    /**
     * @brief Check if a given type is of type `std::array<long, R>` for some arbitrary `R`.
     *
     * @details The shape and the strides of every enda::MemoryArray type is represented as a `std::array<long, R>`, where
     * `R` is the rank of the array.
     *
     * @tparam T Type to check.
     */
    template<class T>
    concept StdArrayOfLong = detail::is_std_array_of_long_v<T>;

    /**
     * @brief Check if a given type is either an arithmetic or complex type.
     * @tparam S Type to check.
     */
    template<typename S>
    concept Scalar = enda::is_scalar_v<S>;

    /**
     * @brief Check if a given type is an instantiation of some other template type.
     *
     * @details See enda::is_instantiation_of for more information.
     *
     * @tparam T Type to check.
     * @tparam TMPLT Template template type to check against.
     */
    template<typename T, template<typename...> class TMPLT>
    concept InstantiationOf = enda::is_instantiation_of_v<TMPLT, T>;

    namespace mem
    {
        struct blk_t;
        enum class AddressSpace;

        /**
         * @brief Check if a given type satisfies the allocator concept.
         *
         * @details Allocators are used to reserve and free memory. Depending on their address space, the memory can either
         * be allocated on the Host (CPU), on the Device (GPU) or on unified memory (see enda::mem::AddressSpace).
         *
         * @note The named <a href="https://en.cppreference.com/w/cpp/named_req/Allocator">C++ Allocator
         * requirements</a> of the standard library are different.
         *
         * @tparam A Type to check.
         */
        template<typename A>
        concept Allocator = requires(A & a)
        {
            {
                a.allocate(size_t {})
            } noexcept -> std::same_as<blk_t>;
            {
                a.allocate_zero(size_t {})
            } noexcept -> std::same_as<blk_t>;
            {
                a.deallocate(std::declval<blk_t>())
            } noexcept;
            {
                A::address_space
            } -> std::same_as<AddressSpace const&>;
        };

        /**
         * @brief Check if a given type satisfies the memory handle concept.
         *
         * @details Memory handles are used to manage memory. They are responsible for providing access to the data which
         * can be located on the stack (CPU), the heap (CPU), the device (GPU) or on unified memory (see
         * enda::mem::AddressSpace).
         *
         * @tparam H Type to check.
         * @tparam T Value type of the handle.
         */
        template<typename H, typename T = typename std::remove_cvref_t<H>::value_type>
        concept Handle = requires(H const& h)
        {
            requires std::is_same_v<typename std::remove_cvref_t<H>::value_type, T>;
            {
                h.is_null()
            } noexcept -> std::same_as<bool>;
            {
                h.data()
            } noexcept -> std::same_as<T*>;
            {
                H::address_space
            } -> std::same_as<AddressSpace const&>;
        };

        /**
         * @brief Check if a given type satisfies the owning memory handle concept.
         *
         * @details In addition to the requirements of the enda::mem::Handle concept, owning memory handles are aware of the
         * size of the memory they manage and can be used to release the memory.
         *
         * @tparam H Type to check.
         * @tparam T Value type of the handle.
         */
        template<typename H, typename T = typename std::remove_cvref_t<H>::value_type>
        concept OwningHandle = Handle<H, T> and requires(H const& h)
        {
            requires not std::is_const_v<typename std::remove_cvref_t<H>::value_type>;
            {
                h.size()
            } noexcept -> std::same_as<long>;
        };

    } // namespace mem

    /**
     * @brief Check if a given type satisfies the array concept.
     *
     * @details An array is a multi-dimensional container of elements. It is characterized by a certain size (number of
     * elements), rank (number of dimensions) and shape (extent of each dimension). Furthermore, it provides access to its
     * elements by overloading the function call operator.
     *
     * Examples of types satisfying this concept are e.g. enda::basic_array or enda::basic_array_view.
     *
     * @note std::array does not satisfy this concept, as it does not have a shape or provides access via the function
     * call operator.
     *
     * @tparam A Type to check.
     */
    template<typename A>
    concept Array = requires(A const& a)
    {
        {
            a.shape()
        } -> StdArrayOfLong;
        {
            a.size()
        } -> std::same_as<long>;
        requires CallableWithLongs<A, get_rank<A>>;
    };

    /**
     * @brief Check if a given type satisfies the memory array concept.
     *
     * @details In addition to the requirements of the enda::Array concept, memory arrays
     * provide access to the underlying memory storage and use an enda::idx_map to specify the
     * layout of the data in memory.
     *
     * An example of a type satisfying the enda::Array but not the enda::MemoryArray concept is enda::expr.
     *
     * @tparam A Type to check.
     */
    template<typename A, typename A_t = std::remove_cvref_t<A>>
    concept MemoryArray = Array<A> && requires(A & a)
    {
        typename A_t::storage_t;
        requires mem::Handle<typename A_t::storage_t>;
        typename A_t::value_type;
        {
            a.data()
        } -> std::same_as<std::conditional_t<std::is_const_v<std::remove_reference_t<A>> || std::is_const_v<typename A_t::value_type>,
                                             const get_value_t<A>,
                                             get_value_t<A>>*>;
        {
            a.indexmap().strides()
        } -> StdArrayOfLong;
    };

    /**
     * @brief Check if a given type is an enda::Array of a certain rank.
     *
     * @tparam A Type to check.
     * @tparam R Rank of the enda::Array type.
     */
    template<typename A, int R>
    concept ArrayOfRank = Array<A> and (get_rank<A> == R);

    /**
     * @brief Check if a given type is an enda::MemoryArray of a certain rank.
     *
     * @tparam A Type to check.
     * @tparam R Rank of the enda::MemoryArray type.
     */
    template<typename A, int R>
    concept MemoryArrayOfRank = MemoryArray<A> and (get_rank<A> == R);

    /**
     * @brief Check if if a given type is either an enda::Array or an enda::Scalar.
     * @tparam AS Type to check.
     */
    template<typename AS>
    concept ArrayOrScalar = Array<AS> or Scalar<AS>;

    /**
     * @brief Check if a given type is a matrix, i.e. an enda::ArrayOfRank<2>.
     * @note The algebra of the type is not checked here (see enda::get_algebra).
     * @tparam M Type to check.
     */
    template<typename M>
    concept Matrix = ArrayOfRank<M, 2>;

    /**
     * @brief Check if a given type is a vector, i.e. an enda::ArrayOfRank<1>.
     * @note The algebra of the type is not checked here (see enda::get_algebra).
     * @tparam V Type to check.
     */
    template<typename V>
    concept Vector = ArrayOfRank<V, 1>;

    /**
     * @brief Check if a given type is a memory matrix, i.e. an enda::MemoryArrayOfRank<2>.
     * @note The algebra of the type is not checked here (see enda::get_algebra).
     * @tparam M Type to check.
     */
    template<typename M>
    concept MemoryMatrix = MemoryArrayOfRank<M, 2>;

    /**
     * @brief Check if a given type is a memory vector, i.e. an enda::MemoryArrayOfRank<1>.
     * @note The algebra of the type is not checked here (see enda::get_algebra).
     * @tparam V Type to check.
     */
    template<typename V>
    concept MemoryVector = MemoryArrayOfRank<V, 1>;

} // namespace enda
