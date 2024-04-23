#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"
#include "prim.h"

typedef struct _Sprite {
    bool              show;
    int               x;
    int               y;
    int               frame;
    struct GfxObject* go;
    uint8_t           blit[2];  // Blit flags, one per screen
    struct Clip       clip[2];  // Clip windows, one per screen
} Sprite;

void sprite_init(Sprite* sprite, struct GfxObject* go, int frame, int x, int y, bool show);
void sprite_hide(const DG* dg, Sprite* sprite);
void sprite_draw(const DG* dg, Sprite* sprite);
