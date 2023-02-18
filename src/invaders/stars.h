#pragma once

#include <inttypes.h>

#include "dg.h"

#define STARS     384
#define SPEED      32
#define X_RANDOM 1022
#define Y_RANDOM 1023
#define Z_RANDOM 1024
#define COLOURS    32

typedef struct _Star {
    int colour;
    int x_off;        /* x offset */
    int32_t y_fix;    /* y position 16:16 fix-point */
    int32_t speed;    /* Pixel speed 16:16 fix-point */
    uint16_t* adr[2]; /* Plot adresses */
    uint8_t pend_rm;  /* Pending to be removed */
} Star;

extern int16_t stars_cmap[COLOURS];

void stars_tables(void);
void stars_hide(DG* dg);
void stars_show(DG* dg);
