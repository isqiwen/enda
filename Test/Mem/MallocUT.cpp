#include <gtest/gtest.h>

#include "Mem/Malloc.hpp"

using namespace enda::mem;

TEST(MemoryAllocationTest, HostMallocFree)
{
    size_t size = 1024;

    void* ptr = malloc<AddressSpace::Host>(size);
    EXPECT_NE(ptr, nullptr);
    free<AddressSpace::Host>(ptr);
}
