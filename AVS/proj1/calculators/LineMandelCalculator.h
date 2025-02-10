/**
 * @file LineMandelCalculator.h
 * @author Jakub Vlk <xvlkja07@stud.fit.vutbr.cz>
 * @brief Implementation of Mandelbrot calculator that uses SIMD paralelization over batch size
 * @date 23.10.2024
 */

#include <BaseMandelCalculator.h>

class LineMandelCalculator : public BaseMandelCalculator {
public:
    LineMandelCalculator(unsigned matrixBaseSize, unsigned limit);

    ~LineMandelCalculator();

    int *calculateMandelbrot();

    inline int mandelbrot(int index, float imag_start, float real_start);


private:
    int *data;
    float *z_real;
    float *z_imag;
    float *x_precomp;
    float *y_precomp;
};