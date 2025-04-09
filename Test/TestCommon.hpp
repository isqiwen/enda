#include <cmath>
#include <gtest/gtest.h>
#include <iostream>
#include <limits>

#ifndef ENDA_DEBUG
    #define ENDA_DEBUG
    #define ENDA_ENFORCE_BOUNDCHECK
#endif

#include "Enda.hpp"

#include "GtestTools.hpp"

using namespace std::complex_literals;
using namespace enda;

#define EXPECT_PRINT(X, Y) \
    { \
        std::stringstream ss; \
        ss << Y; \
        EXPECT_EQ(X, ss.str()); \
    }
