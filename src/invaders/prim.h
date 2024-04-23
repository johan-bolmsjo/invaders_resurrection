#pragma once
/// \file prim.h
///
/// Graphic primitives.
///

#include <stdbool.h>

#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"
#include "libutil/color.h"

struct Clip {
    int x;                      // Position
    int y;
    int w;                      // Visible size
    int h;
    int sx_off;                 // Source X offset
    int sy_off;                 // Source Y offset
};

/// Alpha tables used for mixing the edges of an object with the background.
void prim_module_init(void);

/// Clip graphics frame [gf] to be drawn to coordinates [x] and [y] into
/// [clip]. Returns true if cliped or false if outside screen.
bool clip_gfx_frame(struct Clip* clip, const struct GfxFrame* gf, int x, int y);

/// Blit graphics frame clipped by clip.
void blit_clipped_gfx_frame(const struct MLGraphicsBuffer* dst, const struct Clip* clip, const struct GfxFrame* gf);

/// Blit clipped single coloured box. Used to clear sprites.
void blit_clipped_colour_box(const struct MLGraphicsBuffer* dst, const struct Clip* clip, struct rgb565 color);

/// Blit rectangular block.
void blit_clipped_gfx_box(const struct MLGraphicsBuffer* dst, const struct Clip* clip, const struct rgb565* src);
