#pragma once

#include <stdbool.h>
#include <stdint.h>

/// State for 64-bit pseudo random number generator.
/// The state must be seeded so that it's not everywhere zero.
struct prng64_state {
    union {
        uint64_t l[4];
        uint8_t  b[32];
    };
};

/// Seed 64-bit pseudo random number generator state.
/// Returns true on success.
bool prng64_seed(struct prng64_state* state);

/// Calculate 64-bit pseudo random number based on prior state.
uint64_t prng64_next(struct prng64_state* state);
