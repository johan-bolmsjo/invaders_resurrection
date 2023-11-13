#pragma once

#include <inttypes.h>

#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"
#include "prim.h"

typedef struct _Sprite {
    int        vis;             // Visible when true
    int        x;
    int        y;
    int        frame;
    GfxObject* go;
    uint8_t    blit[2];         // Blit flags, one per screen
    Clip       clip[2];         // Clip windows, one per screen
} Sprite;

void sprite_init(Sprite* sprite, GfxObject* go, int frame, int x, int y, int vis);
void sprite_hide(const DG* dg, Sprite* sprite);
void sprite_show(const DG* dg, Sprite* sprite);
