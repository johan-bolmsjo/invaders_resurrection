#pragma once

#include <stdint.h>

/// Clamp value to within lower and upper bounds.
static inline int
clamp_int(int v, int lower, int upper)
{
    return v < lower ? lower : (v > upper ? upper : v);
}

/// Clamp value to within lower and upper bounds.
static inline double
clamp_double(double v, double lower, double upper)
{
    return v < lower ? lower : (v > upper ? upper : v);
}

/// Calculate the minimum value of a and b.
static inline int
min_int(int a, int b) {
    return a < b ? a : b;
}

/// Calculate the minimum value of a and b.
static inline double
min_double(double a, double b) {
    return a < b ? a : b;
}

/// Calculate the maximum value of a and b.
static inline int
max_int(int a, int b) {
    return a > b ? a : b;
}

/// Calculate the maximum value of a and b.
static inline double
max_double(double a, double b) {
    return a > b ? a : b;
}

/// Round v up to the nearest power of 2 minus 1.
static inline uint32_t
round_up_pow2m1_uint32(uint32_t v) {
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    return v;
}

/// Round v up to the nearest power of 2.
static inline uint32_t
round_up_pow2_uint32(uint32_t v) {
    return round_up_pow2m1_uint32(v - 1) + 1;
}
