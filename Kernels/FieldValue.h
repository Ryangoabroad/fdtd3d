#ifndef FIELD_VALUES_H
#define FIELD_VALUES_H

/**
 * Small values define all field values as float.
 * Full values define all field values as double.
 * Small values decrease memory usage, full values increase accuracy.
 */
#ifdef FLOAT_VALUES
typedef float FPValue;
#endif /* FLOAT_VALUES */
#ifdef DOUBLE_VALUES
typedef double FPValue;
#endif /* DOUBLE_VALUES */
#ifdef LONG_DOUBLE_VALUES
typedef long double FPValue;
#endif /* LONG_DOUBLE_VALUES */

#ifdef COMPLEX_FIELD_VALUES
#include <complex>
typedef std::complex<FPValue> FieldValue;
#else /* COMPLEX_FIELD_VALUES */
typedef FPValue FieldValue;
#endif /* !COMPLEX_FIELD_VALUES */

#ifdef CXX11_ENABLED
#include <cstdint>
#else
#include <stdint.h>
#endif

#ifdef __CUDACC__
#define CUDA_DEVICE __device__
#define CUDA_HOST __host__
#else
#define CUDA_DEVICE
#define CUDA_HOST
#endif

// Type of one-dimensional coordinate.
typedef uint32_t grid_coord;

// Type of three-dimensional coordinate.
typedef uint64_t grid_iter;

// Type of timesteps.
typedef uint32_t time_step;

#endif /* FIELD_VALUES_H */
