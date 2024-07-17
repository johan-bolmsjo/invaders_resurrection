#pragma once
/// \file armada.h
///
/// Armada of invaders (alien bombers).
///

#include <inttypes.h>

#include "bomber.h"
#include "libmedia/libmedia.h"
#include "libutil/prng.h"

enum {
    ArmadaWidth = 10,
    ArmadaHeight = 5,
    ArmadaArea = ArmadaWidth * ArmadaHeight,
};

struct Armada {
    uint8_t y_off;              // Start offset Y-axis

    uint8_t alive_x[ArmadaWidth];  // Alive in X-axis
    uint8_t alive_y[ArmadaHeight];  // Alive in Y-axis

    uint8_t lm;                 // Left most
    uint8_t rm;                 // Right most
    uint8_t tm;                 // Top most
    uint8_t bm;                 // Bottom most

    uint8_t alive;              // Alive ailiens
    uint8_t rows;               // Rows with aliens
    uint8_t vis_c;              // Visible bomber counter
    uint8_t dir_r;              // Direction right
    uint8_t dir_d;              // Direction down

    int8_t  row;                // Active row
    uint8_t row_c;              // Row counter
    uint8_t row_cw;             // Row counter wrap
    uint8_t frac;               // Fraction

    uint8_t kill;               // Kill player?

    uint8_t missiles_max;       // Max number of misiles

    struct Bomber b[ArmadaHeight][ArmadaWidth];
};

extern struct Armada armada; // Used in missiles.c as well

/// Initialize module.
void armada_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state);

/// Reset some values, called from the title screen.
void armada_reset(void);

/// Draw armada to the screen.
void armada_draw(const struct MLGraphicsBuffer* dst);

/// Main bombers function.
void armada_update(void);
