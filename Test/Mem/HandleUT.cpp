#include <cstdlib>
#include <gtest/gtest.h>
#include <utility>
#include <vector>

#include "Mem/Handle.hpp"

using namespace enda::mem;

//==============================================================================
// Test suite for handle_heap
//==============================================================================

TEST(HandleHeapTest, DefaultIsNull)
{
    // Default-constructed handle_heap should be null.
    handle_heap<int> h;
    EXPECT_TRUE(h.is_null());
    EXPECT_EQ(h.size(), 0);
    EXPECT_EQ(h.data(), nullptr);
}

TEST(HandleHeapTest, DoNotInitializeAllocation)
{
    // Allocate memory without initializing it.
    handle_heap<int> h(5, do_not_initialize);
    EXPECT_FALSE(h.is_null());
    EXPECT_EQ(h.size(), 5);
    // Write to the memory and verify.
    for (int i = 0; i < 5; ++i)
    {
        h[i] = i * 10;
    }
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h[i], i * 10);
    }
}

TEST(HandleHeapTest, InitZeroAllocation)
{
    // Allocate memory with zero initialization.
    handle_heap<int> h(5, init_zero);
    EXPECT_FALSE(h.is_null());
    EXPECT_EQ(h.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h[i], 0);
    }
}

TEST(HandleHeapTest, DefaultInitializationNonTrivial)
{
    // For non-trivial types, the default constructor should initialize the objects.
    struct Tracker
    {
        int value;
        Tracker() : value(42) {} // Default value is 42.
        Tracker(const Tracker& other) : value(other.value) {}
        Tracker& operator=(const Tracker& other)
        {
            value = other.value;
            return *this;
        }
        ~Tracker() noexcept {}
    };
    handle_heap<Tracker> h(3);
    EXPECT_FALSE(h.is_null());
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(h[i].value, 42);
    }
}

TEST(HandleHeapTest, CopyConstructorDeepCopy)
{
    handle_heap<int> h1(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        h1[i] = i + 1;
    }
    handle_heap<int> h2(h1);
    EXPECT_FALSE(h2.is_null());
    EXPECT_EQ(h2.size(), h1.size());
    // Modify original and verify the copy is deep.
    h1[0] = 999;
    EXPECT_NE(h1[0], h2[0]);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h2[i], i + 1);
    }
}

TEST(HandleHeapTest, MoveConstructor)
{
    handle_heap<int> h1(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        h1[i] = i * 2;
    }
    handle_heap<int> h2 = std::move(h1);
    // After move, h1 should be null.
    EXPECT_TRUE(h1.is_null());
    // h2 should retain the data.
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h2[i], i * 2);
    }
}

TEST(HandleHeapTest, CopyAssignmentDeepCopy)
{
    handle_heap<int> h1(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        h1[i] = i * 3;
    }
    handle_heap<int> h2;
    h2 = h1;
    EXPECT_FALSE(h2.is_null());
    EXPECT_EQ(h2.size(), h1.size());
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h2[i], i * 3);
    }
    // Ensure deep copy by modifying h1.
    h1[1] = 777;
    EXPECT_NE(h1[1], h2[1]);
}

TEST(HandleHeapTest, MoveAssignment)
{
    handle_heap<int> h1(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        h1[i] = i * 4;
    }
    handle_heap<int> h2;
    h2 = std::move(h1);
    EXPECT_TRUE(h1.is_null());
    EXPECT_EQ(h2.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(h2[i], i * 4);
    }
}

//==============================================================================
// Test suite for handle_stack
//==============================================================================

TEST(HandleStackTest, BasicUsage)
{
    // Create a handle_stack with a fixed size.
    handle_stack<int, 10> hs(10, do_not_initialize);
    // For stack handles, is_null() should always return false.
    EXPECT_FALSE(hs.is_null());
    EXPECT_EQ(hs.size(), 10);
    // Write and verify values.
    for (int i = 0; i < 10; ++i)
    {
        hs[i] = i + 5;
    }
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(hs[i], i + 5);
    }
}

TEST(HandleStackTest, InitZero)
{
    handle_stack<int, 10> hs(10, init_zero);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(hs[i], 0);
    }
}

TEST(HandleStackTest, CopyAndMove)
{
    handle_stack<int, 10> hs1(10, do_not_initialize);
    for (int i = 0; i < 10; ++i)
    {
        hs1[i] = i;
    }
    // Test copy constructor.
    handle_stack<int, 10> hs2 = hs1;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(hs2[i], hs1[i]);
    }
    // Test move constructor.
    handle_stack<int, 10> hs3 = std::move(hs1);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(hs3[i], hs2[i]);
    }
}

//==============================================================================
// Test suite for handle_sso (small string optimization style)
//==============================================================================

TEST(HandleSSOTest, OnStackAllocation)
{
    // When allocated size is within the stack threshold, on_heap() should return false.
    handle_sso<int, 10> sso(5, do_not_initialize);
    EXPECT_FALSE(sso.on_heap());
    EXPECT_EQ(sso.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        sso[i] = i * 7;
    }
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(sso[i], i * 7);
    }
}

TEST(HandleSSOTest, OnHeapAllocation)
{
    // When allocated size exceeds the threshold, on_heap() should return true.
    handle_sso<int, 10> sso(15, do_not_initialize);
    EXPECT_TRUE(sso.on_heap());
    EXPECT_EQ(sso.size(), 15);
    for (int i = 0; i < 15; ++i)
    {
        sso[i] = i * 8;
    }
    for (int i = 0; i < 15; ++i)
    {
        EXPECT_EQ(sso[i], i * 8);
    }
}

TEST(HandleSSOTest, InitZero)
{
    handle_sso<int, 10> sso(5, init_zero);
    EXPECT_EQ(sso.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(sso[i], 0);
    }
}

TEST(HandleSSOTest, CopyAndMove)
{
    handle_sso<int, 10> sso1(7, do_not_initialize);
    for (int i = 0; i < 7; ++i)
    {
        sso1[i] = i + 1;
    }
    // Test copy constructor.
    handle_sso<int, 10> sso2 = sso1;
    EXPECT_EQ(sso2.size(), sso1.size());
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(sso2[i], sso1[i]);
    }
    // Test move constructor.
    handle_sso<int, 10> sso3 = std::move(sso1);
    EXPECT_EQ(sso3.size(), 7);
    for (int i = 0; i < 7; ++i)
    {
        EXPECT_EQ(sso3[i], sso2[i]);
    }
}

//==============================================================================
// Test suite for handle_shared
//==============================================================================

TEST(HandleSharedTest, FromHandleHeap)
{
    // Create a handle_heap and then promote it to a shared handle.
    handle_heap<int> hheap(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        hheap[i] = i + 100;
    }
    handle_shared<int, Host> hshared(hheap);
    EXPECT_FALSE(hshared.is_null());
    EXPECT_EQ(hshared.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(hshared[i], i + 100);
    }
    // Check that the shared pointer's reference count is at least 1.
    EXPECT_GE(hshared.refcount(), 1);
}

//==============================================================================
// Test suite for handle_borrowed
//==============================================================================

TEST(HandleBorrowedTest, BasicUsage)
{
    // Create a handle_heap and then create a borrowed handle from it.
    handle_heap<int> hheap(5, do_not_initialize);
    for (int i = 0; i < 5; ++i)
    {
        hheap[i] = i + 200;
    }
    handle_borrowed<int> hborrowed(hheap);
    EXPECT_FALSE(hborrowed.is_null());
    EXPECT_EQ(hborrowed.data(), hheap.data());
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(hborrowed[i], i + 200);
    }
}

TEST(HandleBorrowedTest, WithOffset)
{
    // Create a handle_heap and then create a borrowed handle with an offset.
    handle_heap<int> hheap(10, do_not_initialize);
    for (int i = 0; i < 10; ++i)
    {
        hheap[i] = i;
    }
    // Borrow starting from offset 5.
    handle_borrowed<int> hborrowed(hheap, 5);
    EXPECT_FALSE(hborrowed.is_null());
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(hborrowed[i], hheap[i + 5]);
    }
}
