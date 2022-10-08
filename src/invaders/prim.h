#pragma once

typedef struct _Clip
{
  int x;       /* Position */
  int y;
  int w;       /* Visible size */
  int h;
  int sx_off;  /* Source X offset */
  int sy_off;  /* Source Y offset */
} Clip;


void
prim_tables ();

int
clip_gfx_frame (Clip *clip, GfxFrame *gf, int x, int y);

void
blit_clipped_gfx_frame (DG *dg, Clip *clip, GfxFrame *gf);

void
blit_clipped_colour_box (DG *dg, Clip *clip, uint16_t colour);

void
blit_clipped_gfx_box (DG *dg, Clip *clip, uint16_t *src);
