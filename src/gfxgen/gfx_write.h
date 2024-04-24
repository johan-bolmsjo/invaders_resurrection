#pragma once

#include "libgfx/libgfx.h"
#include "image.h"

int gfx_create_graphics(struct GfxFrame* f, struct Image* im);
int gfx_create_alpha(struct GfxFrame* f, struct Image* im);
int gfx_create_collision(struct GfxFrame* f, struct Image* im);
int gfx_write(struct GfxObject* o, char* path);
