#pragma once

#include "Mem/Allocators.hpp"
#include "Mem/Handle.hpp"

namespace enda
{
    /**
     * @brief Memory policy using an enda::mem::handle_heap.
     * @tparam Allocator Allocator type to be used.
     */
    template<typename Allocator>
    struct heap_basic
    {
        /**
         * @brief Handle type for the policy.
         * @tparam T Value type of the data.
         */
        template<typename T>
        using handle = mem::handle_heap<T, Allocator>;
    };

    /**
     * @brief Alias template of the enda::heap_basic policy using an enda::mem::mallocator.
     * @tparam AdrSp enda::mem::AddressSpace in which the memory is allocated.
     */
    template<mem::AddressSpace AdrSp = mem::Host>
    using heap = heap_basic<mem::mallocator<AdrSp>>;

    /**
     * @brief Memory policy using an enda::mem::handle_sso.
     * @tparam Size Max. size of the data to store on the stack (number of elements).
     */
    template<std::size_t Size>
    struct sso
    {
        /**
         * @brief Handle type for the policy.
         * @tparam T Value type of the data.
         */
        template<typename T>
        using handle = mem::handle_sso<T, Size>;
    };

    /**
     * @brief Memory policy using an enda::mem::handle_stack.
     * @tparam Size Size of the data (number of elements).
     */
    template<std::size_t Size>
    struct stack
    {
        /**
         * @brief Handle type for the policy.
         * @tparam T Value type of the data.
         */
        template<typename T>
        using handle = mem::handle_stack<T, Size>;
    };

    // Memory policy using an enda::mem::handle_shared.
    struct shared
    {
        /**
         * @brief Handle type for the policy.
         * @tparam T Value type of the data.
         */
        template<typename T>
        using handle = mem::handle_shared<T>;
    };

    /**
     * @brief Memory policy using an enda::mem::handle_borrowed.
     * @tparam AdrSp enda::mem::AddressSpace in which the memory is allocated.
     */
    template<mem::AddressSpace AdrSp = mem::Host>
    struct borrowed
    {
        /**
         * @brief Handle type for the policy.
         * @tparam T Value type of the data.
         */
        template<typename T>
        using handle = mem::handle_borrowed<T, AdrSp>;
    };

} // namespace enda
