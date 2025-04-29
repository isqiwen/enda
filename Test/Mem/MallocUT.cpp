#include "../TestCommon.hpp"

using namespace enda::mem;

TEST(MemoryAllocationTest, HostMallocFree)
{
    size_t size = 1024;

    void* ptr = malloc<AddressSpace::Host>(size);
    EXPECT_NE(ptr, nullptr);
    free<AddressSpace::Host>(ptr);
}

TEST(MemoryAllocationTest, HostMallocFreeAlignment)
{
    size_t size = 1024;

    void* ptr = malloc<AddressSpace::Host, 128>(size);
    EXPECT_NE(ptr, nullptr);
    EXPECT_EQ(is_aligned(ptr, 128), true);
    free<AddressSpace::Host, 128>(ptr);
}
