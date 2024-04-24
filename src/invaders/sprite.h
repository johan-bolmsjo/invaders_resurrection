#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"
#include "prim.h"

struct Sprite {
    bool              show;
    int               x;
    int               y;
    int               frame;
    struct GfxObject* gfx_obj;
};

void sprite_init(struct Sprite* sprite, struct GfxObject* go, int frame, int x, int y, bool show);
void sprite_draw(const struct MLGraphicsBuffer* dst, const struct Sprite* sprite);
