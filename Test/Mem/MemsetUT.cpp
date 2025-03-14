#include <gtest/gtest.h>

#include "Mem/Memset.hpp"

using namespace enda::mem;

TEST(MemsetTest, HostMemset)
{
    constexpr size_t SIZE = 128;
    unsigned char    buffer[SIZE];

    std::memset(buffer, 255, SIZE);
    EXPECT_EQ(buffer[0], 255);

    memset<Host>(buffer, 0, SIZE);

    for (size_t i = 0; i < SIZE; ++i)
    {
        EXPECT_EQ(buffer[i], 0);
    }
}

TEST(MemsetTest, HostMemset2D)
{
    constexpr size_t WIDTH  = 16;
    constexpr size_t HEIGHT = 8;
    constexpr size_t PITCH  = 20;
    unsigned char    buffer[PITCH * HEIGHT];

    std::memset(buffer, 255, PITCH * HEIGHT);

    memset2D<Host>(buffer, PITCH, 0, WIDTH, HEIGHT);

    for (size_t row = 0; row < HEIGHT; ++row)
    {
        for (size_t col = 0; col < WIDTH; ++col)
        {
            EXPECT_EQ(buffer[row * PITCH + col], 0);
        }
        for (size_t col = WIDTH; col < PITCH; ++col)
        {
            EXPECT_EQ(buffer[row * PITCH + col], 255);
        }
    }
}
