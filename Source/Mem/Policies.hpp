// Copyright (c) 2019-2023 Simons Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0.txt
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Authors: Olivier Parcollet, Nils Wentzell

/**
 * @file
 * @brief Defines various memory handling policies.
 */

#pragma once

#include "./allocators.hpp"
#include "./handle.hpp"

namespace nda {

  /**
  * @addtogroup mem_pols
  * @{
  */

#ifdef NDA_TEST_DEFAULT_ALLOC_MBUCKET // for testing only: use multi_bucket allocator
  template <typename Allocator =
               mem::segregator<8 * NDA_TEST_DEFAULT_ALLOC_MBUCKET, mem::multi_bucket<8 * NDA_TEST_DEFAULT_ALLOC_MBUCKET>, mem::mallocator>>
#else // normal case
  /**
   * @brief Memory policy using an nda::mem::handle_heap.
   * @tparam Allocator Allocator type to be used.
   */
  template <typename Allocator>
#endif
  struct heap_basic {
    /**
     * @brief Handle type for the policy.
     * @tparam T Value type of the data.
     */
    template <typename T>
#ifdef NDA_TEST_DEFAULT_ALLOC_SSO // for testing only: use a handle_sso if possible
    using handle =
       std::conditional_t<std::is_copy_constructible_v<T>, mem::handle_sso<T, NDA_TEST_DEFAULT_ALLOC_SSO>, mem::handle_heap<T, Allocator>>;
#else
    using handle = mem::handle_heap<T, Allocator>;
#endif
  };

  /**
   * @brief Alias template of the nda::heap_basic policy using an nda::mem::mallocator.
   * @tparam AdrSp nda::mem::AddressSpace in which the memory is allocated.
   */
  template <mem::AddressSpace AdrSp = mem::Host>
  using heap = heap_basic<mem::mallocator<AdrSp>>;

  /**
   * @brief Memory policy using an nda::mem::handle_sso.
   * @tparam Size Max. size of the data to store on the stack (number of elements).
   */
  template <size_t Size>
  struct sso {
    /**
     * @brief Handle type for the policy.
     * @tparam T Value type of the data.
     */
    template <typename T>
    using handle = mem::handle_sso<T, Size>;
  };

  /**
   * @brief Memory policy using an nda::mem::handle_stack.
   * @tparam Size Size of the data (number of elements).
   */
  template <size_t Size>
  struct stack {
    /**
     * @brief Handle type for the policy.
     * @tparam T Value type of the data.
     */
    template <typename T>
    using handle = mem::handle_stack<T, Size>;
  };

  /// Memory policy using an nda::mem::handle_shared.
  struct shared {
    /**
     * @brief Handle type for the policy.
     * @tparam T Value type of the data.
     */
    template <typename T>
    using handle = mem::handle_shared<T>;
  };

  /**
   * @brief Memory policy using an nda::mem::handle_borrowed.
   * @tparam AdrSp nda::mem::AddressSpace in which the memory is allocated.
   */
  template <mem::AddressSpace AdrSp = mem::Host>
  struct borrowed {
    /**
     * @brief Handle type for the policy.
     * @tparam T Value type of the data.
     */
    template <typename T>
    using handle = mem::handle_borrowed<T, AdrSp>;
  };

  /** @} */

} // namespace nda
