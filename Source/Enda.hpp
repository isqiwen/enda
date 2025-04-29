/**
 * @file Enda.hpp
 *
 * @brief Includes all relevant headers for the core nda library.
 */

#pragma once

#if (defined(_WIN32) || defined(_WIN64)) && !defined(ENDA_NO_WINDOWS_H)
    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>
#endif

// Remove a few macros
#undef min
#undef max

// Debug
// #define ENDA_DEBUG

#ifdef ENDA_DEBUG
    #define ENDA_ENFORCE_BOUNDCHECK
#endif

#include "Accessors.hpp"
#include "Algorithms.hpp"
#include "Arithmetic.hpp"
#include "BasicArray.hpp"
#include "BasicArrayView.hpp"
#include "BasicFunctions.hpp"
#include "Concepts.hpp"
#include "Declarations.hpp"
#include "Device.hpp"
#include "Exceptions.hpp"
#include "GroupIndices.hpp"
#include "Iterators.hpp"
#include "Layout.hpp"
#include "LayoutTransforms.hpp"
#include "Macros.hpp"
#include "Map.hpp"
#include "MappedFunctions.hpp"
#include "MatrixFunctions.hpp"
#include "Mem.hpp"
#include "Print.hpp"
#include "StdUtil.hpp"
#include "Traits.hpp"
