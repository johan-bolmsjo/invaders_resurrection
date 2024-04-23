#pragma once

#include <inttypes.h>

enum {
    // Max values for red, green and blue pixel components.
    // These also works as bit masks.
    max_r5 = 0x1f,
    max_g6 = 0x3f,
    max_b5 = 0x1f,

    // Left shift of red, green and blue pixel components.
    shift_rgb565_b = 0,
    shift_rgb565_g = shift_rgb565_b + 5,
    shift_rgb565_r = shift_rgb565_g + 6,
};

struct rgb {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct rgb565 {
    uint16_t v;
};

/// Pack separate RGB components into a RGB565 value.
/// The input RGB components are expected to be pre scaled as RGB565.
static inline struct rgb565
pack_rgb565(struct rgb c) {
    return (struct rgb565){(c.r & max_r5) << shift_rgb565_r | (c.g & max_g6) << shift_rgb565_g | (c.b & max_b5)};
}

/// Unpack RGB565 value into separate RGB components.
/// The unpacked RGB components are not scaled to RGB888.
static inline struct rgb
unpack_rgb565(struct rgb565 c) {
    return (struct rgb) {
        .r = (c.v >> shift_rgb565_r) & max_r5,
        .g = (c.v >> shift_rgb565_g) & max_g6,
        .b = (c.v >> shift_rgb565_b) & max_b5,
    };
}

/// Add two RGB565 values witout color component saturation.
static inline struct rgb565
add_rgb565(struct rgb565 c0, struct rgb565 c1) {
    return  (struct rgb565){c0.v + c1.v};
}

static inline struct rgb
rgb565_color_white(void) {
    return (struct rgb){.r = max_r5, .g = max_g6, .b = max_b5};
}

static inline struct rgb
rgb565_color_black(void) {
    return (struct rgb){.r = 0, .g = 0, .b = 0};
}
