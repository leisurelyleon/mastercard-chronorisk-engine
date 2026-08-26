#pragma once
#include <immintrin.h>
#include <vector>
#include <cstddef>
#include <cmath>

namespace chronorisk::math {

class SIMD_MathCore {
public:
    // Computes dot product of two aligned float vectors using AVX2 FMA instructions
    static float VectorDotProductAVX2(const float* a, const float* b, size_t count) {
        size_t i = 0;
        __m256 sum256 = _mm256_setzero_ps();

        for (; i + 8 <= count; i += 8) {
            __m256 va = _mm256_loadu_ps(a + i);
            __m256 vb = _mm256_loadu_ps(b + i);
            sum256 = _mm256_fmadd_ps(va, vb, sum256);
        }

        // Horizontal add of the 256-bit register
        alignas(32) float buffer[8];
        _mm256_storeu_ps(buffer, sum256);
        float total = buffer[0] + buffer[1] + buffer[2] + buffer[3] +
                      buffer[4] + buffer[5] + buffer[6] + buffer[7];

        // Process trailing elements
        for (; i < count; ++i) {
            total += a[i] * b[i];
        }

        return total;
    }

    // Vectorized log-return calculation across time steps
    static void VectorizedLogReturns(const float* prices, float* out_returns, size_t count) {
        for (size_t i = 0; i < count - 1; ++i) {
            out_returns[i] = std::log(prices[i + 1] / prices[i]);
        }
    }
};

} // namespace chronorisk::math
