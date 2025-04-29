/**
 * @file Padding.hpp
 *
 * @brief Provides a method to compute the minimal extra padding in the fastest changing dimension for 2D or 3D array.
 */

#pragma once

#include <numeric>
#include <vector>

#ifdef _WIN32
    #include <intrin.h>
#else
    #include <cpuid.h>
#endif

namespace enda
{

    // Extended Euclidean algorithm:
    // Computes and returns the greatest common divisor (gcd) of a and b,
    // while setting x and y such that: a * x + b * y == gcd(a, b)
    constexpr int ext_euc(int a, int b, int& x, int& y)
    {
        if (b == 0)
        {
            x = 1, y = 0;
            return a;
        }

        int d = ext_euc(b, a % b, y, x);
        y -= a / b * x;

        return d;
    }

    // Compute the modular inverse of 'a' modulo 'm'.
    // If there is no modular inverse (i.e., a and m are not coprime),
    // returns -1.
    constexpr int mod_inverse(int a, int m)
    {
        int x, y;
        int g = ext_euc(a, m, x, y);

        if (g != 1)
        {
            return -1;
        }

        x %= m;
        if (x < 0)
        {
            x += m;
        }

        return x;
    }

    // Structure to hold cache information
    struct cache_info
    {
        unsigned int level;      // Cache level (e.g., 1 for L1 cache)
        unsigned int cacheType;  // Cache type: 1 = Data Cache, 2 = Instruction Cache, 3 = Unified Cache
        unsigned int ways;       // Cache associativity (number of ways)
        unsigned int partitions; // Number of physical line partitions (usually 1)
        unsigned int lineSize;   // Cache line size in bytes
        unsigned int sets;       // Number of cache sets
        unsigned int size;       // Total cache size in bytes
    };

    // Function to get L1 Data Cache information using CPUID (leaf 4).
    // This implementation works on both Windows and Linux.
    inline cache_info get_L1_data_cache_info()
    {
        cache_info   info = {};
        unsigned int eax, ebx, ecx, edx;
        int          subleaf = 0;
        bool         found   = false;

#ifdef _WIN32
        int cpuInfo[4] = {0};
        // Iterate over CPUID leaf 4 subleaves
        while (true)
        {
            __cpuidex(cpuInfo, 4, subleaf);
            eax = cpuInfo[0];
            // If the cache type (lowest 5 bits of EAX) is 0, no more cache information is available.
            if ((eax & 0x1F) == 0)
                break;
            // Check if this subleaf is for L1 data cache:
            // Cache Level (EAX[7:5]) should be 1 and Cache Type (EAX[4:0]) must be 1 (Data Cache)
            if ((((eax >> 5) & 0x7) == 1) && ((eax & 0x1F) == 1))
            {
                found = true;
                break;
            }
            subleaf++;
        }
        if (!found)
            throw std::runtime_error("L1 Data Cache info not found");
        eax = cpuInfo[0];
        ebx = cpuInfo[1];
        ecx = cpuInfo[2];
        edx = cpuInfo[3];
#else
        // On Linux, use __get_cpuid_count to get CPUID leaf 4 information
        while (__get_cpuid_count(4, subleaf, &eax, &ebx, &ecx, &edx))
        {
            if ((eax & 0x1F) == 0)
                break; // No more cache information available
            if ((((eax >> 5) & 0x7) == 1) && ((eax & 0x1F) == 1))
            {
                found = true;
                break;
            }
            subleaf++;
        }
        if (!found)
            throw std::runtime_error("L1 Data Cache info not found");
#endif

        // Parse the CPUID leaf-4 results:
        // EAX bits 4:0 store the Cache Type Field (1 = Data Cache)
        info.cacheType = eax & 0x1F;
        // EAX bits 7:5 store the cache level (e.g., 1 means L1)
        info.level = (eax >> 5) & 0x7;
        // EBX bits 31:22 contain the "ways of associativity" minus 1
        info.ways = ((ebx >> 22) & 0x3FF) + 1;
        // EBX bits 21:12 contain the "number of physical line partitions" minus 1
        info.partitions = ((ebx >> 12) & 0x3FF) + 1;
        // EBX bits 11:0 store the "System Coherency Line Size" minus 1 (in bytes)
        info.lineSize = (ebx & 0xFFF) + 1;
        // ECX holds the "number of sets" minus 1
        info.sets = ecx + 1;
        // Total cache size: ways * partitions * lineSize * sets (in bytes)
        info.size = info.ways * info.partitions * info.lineSize * info.sets;
        return info;
    }

    //------------------------------------------------------------------------------
    // compute_padding_2d_set_assoc Function Implementation
    //------------------------------------------------------------------------------
    //
    // This function computes the minimal extra padding (in elements)
    // in the fastest changing dimension (horizontal direction) for a 2D data tile
    // so that when mapped to a set-associative cache the conflict count on the cache
    // sets is less than the associativity A.
    // Parameters:
    //   S   - Number of cache sets (e.g., 8)
    //   A   - Cache associativity (number of cache lines allowed per set)
    //   D2  - Tile height (number of rows in the tile)
    //   D1  - Tile width (number of columns in the tile)
    //   M1  - Unpadded array width (in elements)
    //   B   - Cache block (line) size (in elements)
    //
    // The algorithm proceeds as follows:
    //   1. For each candidate conflict vector (i2, i1) within the tile (excluding (0,0)),
    //      where i2 ranges from 0 to D2-1 and i1 ranges from -(D1+B)/B to (D1-B)/B:
    //      a. Compute c = gcd(i2, S). Only consider the candidate if i1 is divisible by c.
    //      b. Compute the modular inverse of (i2/c) modulo (S/c) (if S/c > 1).
    //      c. For each i0 in [0, c), compute the candidate residual index:
    //             index = ( (-i1 * inv) mod (S/c) ) + i0*(S/c)
    //         and increment the conflict count at that index.
    //   2. Finally, examine candidates corresponding to (M1 + i0*B) modulo S for i0 from 0 to S-1.
    //      Return the smallest i0*B for which the conflict count is less than A.
    // -----------------------------------------------------------------------------
    constexpr int compute_padding_2d_set_assoc(int S, int A, int D2, int D1, int M1, int B)
    {
        // conflict[i] holds the number of conflicts for candidate residual index i.
        std::vector<int> conflict(S, 0);
        for (int i2 = 0; i2 < D2; ++i2)
        {
            // Loop over each row of the tile.
            int lower = -(D1 + B) / B;
            int upper = (D1 - B) / B;
            for (int i1 = lower; i1 <= upper; ++i1)
            {
                // Skip the reference element (0,0).
                if (i2 == 0 && i1 == 0)
                {
                    continue;
                }
                // Compute c = gcd(i2, S).
                int c = std::gcd(i2, S);
                // Only consider conflict vectors where i1 is divisible by c.
                if (i1 % c != 0)
                {
                    continue;
                }
                int inv   = 0;
                int denom = S / c;
                if (denom > 1)
                {
                    inv = mod_inverse(i2 / c, denom);

                    if (inv == -1)
                    {
                        continue;
                    }
                }
                else
                {
                    inv = 0;
                }
                // For each sub-index i0, compute the candidate index and update conflict count.
                for (int i0 = 0; i0 < c; ++i0)
                {
                    int modBase = denom;
                    int v       = (-i1 * inv) % modBase;
                    if (v < 0)
                    {
                        v += modBase;
                    }
                    int index = v + i0 * modBase;
                    if (index >= 0 && index < S)
                    {
                        conflict[index]++;
                    }
                }
            }
        }

        // Choose the minimal padding i0 * B such that the candidate residual
        // index (M1 + i0 * B) modulo S has a conflict count less than the associativity A.
        for (int i0 = 0; i0 < S; ++i0)
        {
            int index = (M1 + i0 * B) % S;
            if (conflict[index] < A)
            {
                return i0 * B;
            }
        }

        // If no candidate is found (theoretically, this should not happen), return 0.
        return 0;
    }

} // namespace enda
