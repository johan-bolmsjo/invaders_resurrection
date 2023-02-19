#pragma once

// Clamp integer to within lower and upper bounds.
static inline int
clamp_int(int v, int lower, int upper)
{
    return v < lower ? lower : (v > upper ? upper : v);
}
