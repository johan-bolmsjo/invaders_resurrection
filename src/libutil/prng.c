// prng64 is based on https://prng.di.unimi.it/xoshiro256starstar.c
// which is dedicated to the public domain.

#include "prng.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

static inline uint64_t
prng64_rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static inline bool
read_state_from_file(struct prng64_state* state, const char* filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return false;
    }

    ssize_t r = read(fd, state->b, sizeof state->b);
    if (r != sizeof state->b) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool
prng64_seed(struct prng64_state* state)
{
    // Default seed is better than nothing in case of miss-use.
    // Apparently the prng does not like an all zero seed.
    const struct prng64_state default_seed = {
        .l = {
            0x3d96886378e7e536,
            0xf03f57692c390c9e,
            0x8ba8eef5a66304c0,
            0x7db75bc46ac3d29c,
        }
    };
    const struct prng64_state zero_seed = {{{0}}};

    // TODO(jb): This is specific to some Unix like operating systems.
    if (!read_state_from_file(state, "/dev/random")) {
        *state = default_seed;
        return false;
    }

    if (!memcmp(state, &zero_seed, sizeof(*state))) {
        *state = default_seed;
    }

    return true;
}

uint64_t
prng64_next(struct prng64_state* state)
{
    const uint64_t result = prng64_rotl(state->l[1] * 5, 7) * 9;

    const uint64_t t = state->l[1] << 17;

    state->l[2] ^= state->l[0];
    state->l[3] ^= state->l[1];
    state->l[1] ^= state->l[2];
    state->l[0] ^= state->l[3];

    state->l[2] ^= t;

    state->l[3] = prng64_rotl(state->l[3], 45);

    return result;
}

double
prng64_next_double(struct prng64_state* state)
{
    const int precision = 53;
    return (double)(prng64_next(state) >> (64 - precision)) / (double)(1ULL << precision);
}
