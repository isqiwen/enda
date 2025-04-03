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
