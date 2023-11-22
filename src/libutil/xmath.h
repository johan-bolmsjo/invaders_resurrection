#pragma once

/// Clamp integer to within lower and upper bounds.
static inline int
clamp_int(int v, int lower, int upper)
{
    return v < lower ? lower : (v > upper ? upper : v);
}

/// Calculate the minimum value of a and b.
static inline int
min_int(int a, int b) {
    return a < b ? a : b;
}

/// Calculate the maximum value of a and b.
static inline int
max_int(int a, int b) {
    return a > b ? a : b;
}
