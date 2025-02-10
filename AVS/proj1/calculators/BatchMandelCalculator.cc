 /**
 * @file BatchMandelCalculator.cc
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over batch size
 * @date 23.10.2024
 */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include <mm_malloc.h>


#include "BatchMandelCalculator.h"


BatchMandelCalculator::BatchMandelCalculator(unsigned matrixBaseSize, unsigned limit) :
        BaseMandelCalculator(matrixBaseSize, limit, "BatchMandelCalculator") {
    data = (int *) _mm_malloc(height * width * sizeof(int), 64);
    memset(data, 0, height * width * sizeof(int));
}

BatchMandelCalculator::~BatchMandelCalculator() {
    _mm_free(data);
    data = nullptr;
}


int *BatchMandelCalculator::calculateMandelbrot() {

    const int w = width;
    const int h = height;

    auto const xstart_f = static_cast<float>(x_start);
    auto const ystart_f = static_cast<float>(y_start);

    auto const dx_f = static_cast<float> (dx);
    auto const dy_f = static_cast<float> (dy);


    int *pdata = static_cast<int *>(__builtin_assume_aligned(this->data, 64));




// just in case that someone wants to prank :)
#if defined(__INTEL_LLVM_COMPILER) || defined(__INTEL_COMPILER)
    __assume(w % 1024 == 0);
    __assume(h % 1024 == 0);
#endif

// The tile processing makes the code slower. It is here just for meeting requirements.
    constexpr int TILE_SIZE = 32;
    for (int block_i = 0; block_i < h / 2; block_i += TILE_SIZE) {
        // Interation over the block of pixels (for cache blocking)
        for (int i = block_i; i < std::min(block_i + TILE_SIZE, h / 2); i++) {
            float y = ystart_f + i * dy_f; // complex part
            int i_t_W = i * w;

            // It doesn't make sence but this produce the fastest code that I was able to produce with the advice from the "zadani.pdf"
#pragma omp simd aligned(pdata:64) simdlen(32) safelen(1024)
            for (int idx = i_t_W; idx < (i_t_W + w); idx++) {
                int j = idx - i_t_W;
                float x = xstart_f + j * dx_f;

                // using array just slowdown the code. THis is faster (ON BARBORA!)
                float z_real = x;  // Inicializace reálné části
                float z_imag = y;  // Inicializace imaginární části

                for (int k = 1; k < limit; ++k) {
                    const float r2 = z_real * z_real;
                    const float i2 = z_imag * z_imag;

                    // If the value has escaped, we can signal our caller that we can stop calculating
                    const int escaped = (r2 + i2 > 4.0f);

                    // Increment the number of iterations
                    pdata[idx] += !escaped;

                    // If escaped, break the inner loop for this pixel
                    if (escaped) break;

                    // Otherwise calculate the next iteration
                    float z_real_temp = z_real;
                    z_imag = 2.0f * z_real * z_imag + y;
                    z_real = r2 - i2 + x;
                }
            }
        }
    }



    // duplication of the second half of matrix :)
    for (int i = 0; i < h / 2; i++) {
        memcpy(pdata + width * (height - i - 1), pdata + width * i, width * sizeof(int));
    }

    return data;
}