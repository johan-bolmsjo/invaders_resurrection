#pragma once
/// \file prim.h
///
/// Graphic primitives.
///

#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"

typedef struct _Clip {
    int x;                      // Position
    int y;
    int w;                      // Visible size
    int h;
    int sx_off;                 // Source X offset
    int sy_off;                 // Source Y offset
} Clip;

/// Alpha tables used for mixing the edges of an object with the background.
void prim_module_init(void);

/// Returns 0 if cliped or 1 if outside screen.
int clip_gfx_frame(Clip* clip, GfxFrame* gf, int x, int y);

/// Blit graphics frame clipped by clip.
void blit_clipped_gfx_frame(const DG* dg, Clip* clip, GfxFrame* gf);

/// Blit clipped single coloured box. Used to clear sprites.
/// TODO(jb): Use struct rgb565 for colour
void blit_clipped_colour_box(const DG* dg, Clip* clip, uint16_t colour);

/// Blit rectangular block.
/// TODO(jb): Use struct rgb565 for src
void blit_clipped_gfx_box(const DG* dg, Clip* clip, uint16_t* src);
