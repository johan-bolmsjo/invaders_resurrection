/* Graphic primitives.
 */

#include "prim.h"

#include <stdio.h>
#include <inttypes.h>
#include <inttypes.h>

// TODO(jb): Use struct rgb565; extend color.h as appropriate
static uint16_t ALPHA_RED[32 * 16];
static uint16_t ALPHA_GREEN[64 * 16];
static uint16_t ALPHA_BLUE[32 * 16];

/* Alpha tables used for mixing the edges of an object with the
 * background. 0 is opaque, 16 is totaly transparent.
 */

void
prim_tables(void)
{
    int i, j, atom, v;

    atom = 16;
    for (j = 0; j < 16; j++) {
        v = 0;
        for (i = 0; i < 32; i++) {
            ALPHA_RED[j << 5 | i] = (v >> 4) << 11;
            v += atom;
        }
        v = 0;
        for (i = 0; i < 64; i++) {
            ALPHA_GREEN[j << 6 | i] = (v >> 4) << 5;
            v += atom;
        }
        v = 0;
        for (i = 0; i < 32; i++) {
            ALPHA_BLUE[j << 5 | i] = (v >> 4);
            v += atom;
        }
        atom--;
    }
}

/* Returns 0 if cliped or 1 if outside screen.
 */

int
clip_gfx_frame(Clip* clip, GfxFrame* gf, int x, int y)
{
    int x2, y2;

    x -= gf->x_off;
    y -= gf->y_off;
    x2 = x + gf->width - 1;
    y2 = y + gf->height - 1;

    if (x >= DG_XRES || x2 < 0 || y >= DG_YRES || y2 < 0)
        return 1;

    if (x < 0) {
        clip->sx_off = -x;
        x = 0;
    } else {
        clip->sx_off = 0;
    }
    if (x2 >= DG_XRES)
        x2 = DG_XRES - 1;

    if (y < 0) {
        clip->sy_off = -y;
        y = 0;
    } else {
        clip->sy_off = 0;
    }
    if (y2 >= DG_YRES)
        y2 = DG_YRES - 1;

    clip->x = x;
    clip->y = y;
    clip->w = x2 - x + 1;
    clip->h = y2 - y + 1;

    return 0;
}

/* For frames with alpha channel.
 */

static void
blit_clipped_gfx_frame_with_alpha(DG* dg, Clip* clip, GfxFrame* gf)
{
    int w, h, s_stride, d_stride, sa, da, sp, dp;
    uint8_t* alpha;
    uint16_t *src, *dst; // TODO(jb): Use struct rgb565

    s_stride = gf->width - clip->w;
    d_stride = DG_XRES - clip->w;

    alpha = gf->alpha + clip->sx_off + clip->sy_off * gf->width;
    src = gf->graphics + clip->sx_off + clip->sy_off * gf->width;
    dst = dg->adr[dg->hid] + clip->x + clip->y * DG_XRES;

    for (h = clip->h; h > 0; h--) {
        for (w = clip->w; w > 0; w--) {
            sa = *alpha++;
            if (sa == 0) {
                *dst++ = *src++;
            } else {
                if (sa == 16) {
                    src++;
                    dst++;
                } else {
                    da = 16 - sa;
                    sp = *src++;
                    dp = *dst;

                    /* Assembler? GCC produces ugly shit even on ARM
                     * wich have realy nice shift capabilities:(
                     *
                     * Use 2 MB table? 65536 * 2 * 16.
                     */
                    sp = ALPHA_RED[(sa << 5) | (sp >> 11)] |
                         ALPHA_GREEN[(sa << 6) | ((sp >> 5) & 0x3F)] |
                         ALPHA_BLUE[(sa << 5) | (sp & 0x1F)];

                    dp = ALPHA_RED[(da << 5) | (dp >> 11)] |
                         ALPHA_GREEN[(da << 6) | ((dp >> 5) & 0x3F)] |
                         ALPHA_BLUE[(da << 5) | (dp & 0x1F)];

                    *dst++ = sp + dp;
                }
            }
        }

        alpha += s_stride;
        src += s_stride;
        dst += d_stride;
    }
}

/* For frames without alpha channel.
 */

static void
blit_clipped_gfx_frame_without_alpha(DG* dg, Clip* clip, GfxFrame* gf)
{
    int w, h, s_stride, d_stride;
    uint16_t *src, *dst, v; // TODO(jb): Use struct rgb565

    s_stride = gf->width - clip->w;
    d_stride = DG_XRES - clip->w;
    src = gf->graphics + clip->sx_off + clip->sy_off * gf->width;
    dst = dg->adr[dg->hid] + clip->x + clip->y * DG_XRES;

    for (h = clip->h; h > 0; h--) {
        for (w = clip->w; w > 0; w--) {
            v = *src++;
            if (v)
                *dst = v;
            dst++;
        }

        src += s_stride;
        dst += d_stride;
    }
}

/* Blit graphics frame clipped by clip.
 */

void
blit_clipped_gfx_frame(DG* dg, Clip* clip, GfxFrame* gf)
{
    if (gf->alpha)
        blit_clipped_gfx_frame_with_alpha(dg, clip, gf);
    else
        blit_clipped_gfx_frame_without_alpha(dg, clip, gf);
}

/* Blit clipped single coloured box,
 * used to clear sprites.
 */
// TODO(jb): Use struct rgb565 for colour
void
blit_clipped_colour_box(DG* dg, Clip* clip, uint16_t colour)
{
    int w, h, d_stride;
    uint16_t* dst; // TODO(jb): Use struct rgb565

    d_stride = DG_XRES - clip->w;
    dst = dg->adr[dg->hid] + clip->x + clip->y * DG_XRES;

    for (h = clip->h; h > 0; h--) {
        for (w = clip->w; w > 0; w--)
            *dst++ = colour;

        dst += d_stride;
    }
}

/* Blit rectangular block.
 */
// TODO(jb): Use struct rgb565 for src
void
blit_clipped_gfx_box(DG* dg, Clip* clip, uint16_t* src)
{
    int w, h, d_stride;
    uint16_t* dst; // TODO(jb): Use struct rgb565

    d_stride = DG_XRES - clip->w;
    dst = dg->adr[dg->hid] + clip->x + clip->y * DG_XRES;

    for (h = clip->h; h > 0; h--) {
        for (w = clip->w; w > 0; w--)
            *dst++ = *src++;

        dst += d_stride;
    }
}
