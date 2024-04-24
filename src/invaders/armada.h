#pragma once
/// \file armada.h
///
/// Armada of invaders (alien bombers).
///

#include <inttypes.h>

#include "bomber.h"
#include "libmedia/libmedia.h"

#define ARMADA_X  10
#define ARMADA_Y  5
#define ARMADA_XY (ARMADA_X * ARMADA_Y)

struct Armada {
    uint8_t y_off;              // Start offset Y-axis

    uint8_t alive_x[ARMADA_X];  // Alive in X-axis
    uint8_t alive_y[ARMADA_Y];  // Alive in Y-axis

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

    struct Bomber b[ARMADA_Y][ARMADA_X];
};

extern struct Armada armada; // Used in missiles.c as well

/// Initialize module.
void armada_module_init(void);

/// Reset some values, called from the title screen.
void armada_reset(void);

/// Draw armada on the screen.
void armada_draw(const struct MLGraphicsBuffer* dst);

/// Main bombers function.
void armada_update(void);
