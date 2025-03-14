#include <gtest/gtest.h>

#include "Mem/Memcpy.hpp"

using namespace enda::mem;

TEST(MemcpyTest, HostToHost_CopySmallBuffer)
{
    const size_t  size = 16;
    unsigned char src[size];
    unsigned char dest[size];
    std::memset(src, 42, size);
    std::memset(dest, 0, size);

    memcpy<Host, Host>(dest, src, size);
    EXPECT_EQ(std::memcmp(src, dest, size), 0);
}

TEST(MemcpyTest, HostToHost_CopyZeroBytes)
{
    unsigned char src[16];
    unsigned char dest[16];
    std::memset(src, 42, 16);
    std::memset(dest, 0, 16);

    memcpy<Host, Host>(dest, src, 0);
    EXPECT_EQ(std::memcmp(dest, src, 16), -1);
}

TEST(Memcpy2DTest, HostToHost_CopyMatrix)
{
    const size_t width  = 4;
    const size_t height = 3;
    const size_t pitch  = 8;

    unsigned char src[height * pitch];
    unsigned char dest[height * pitch];
    std::memset(src, 42, height * pitch);
    std::memset(dest, 0, height * pitch);

    memcpy2D<Host, Host>(dest, pitch, src, pitch, width, height);
    for (size_t i = 0; i < height; ++i)
    {
        EXPECT_EQ(std::memcmp(dest + i * pitch, src + i * pitch, width), 0);
    }
}
