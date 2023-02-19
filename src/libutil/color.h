#pragma once

#include <inttypes.h>

enum {
    // Max values for red, green and blue pixel components.
    // These also works as bit masks.
    max_r = 0x1f,
    max_g = 0x3f,
    max_b = 0x1f,

    // Left shift of red, green and blue pixel components.
    shift_b = 0,
    shift_g = shift_b + 5,
    shift_r = shift_g + 6,
};

struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct rgb565 {
    uint16_t v;
};

static inline struct rgb565
pack_rgb565(struct rgb c) {
    return (struct rgb565){(c.r & max_r) << shift_r | (c.g & max_g) << shift_g | (c.b & max_b)};
}

static inline struct rgb
unpack_rgb565(struct rgb565 c) {
    return (struct rgb) {
        .r = (c.v >> shift_r) & max_r,
        .g = (c.v >> shift_g) & max_g,
        .b = (c.v >> shift_b) & max_b,
    };
}

static inline struct rgb
rgb_white(void) {
    return (struct rgb){.r = max_r, .g = max_g, .b = max_b};
}

static inline struct rgb
rgb_black(void) {
    return (struct rgb){.r = 0, .g = 0, .b = 0};
}
