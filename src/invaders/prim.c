#include "prim.h"
#include "libmedia/libmedia.h"

#include <stdio.h>
#include <inttypes.h>
#include <inttypes.h>

// Alpha tables used for mixing the edges of an object with the background.
//
// Value 0 is opaque and 16 is transparent.
// Values 0 and 16 are never indexed into these tables.
//
static struct {
    struct MLRectDim screen_dim;

    struct rgb565 alpha_r_lut[16 * 32];
    struct rgb565 alpha_g_lut[16 * 64];
    struct rgb565 alpha_b_lut[16 * 32];
} prim_module;
#define M prim_module

void
prim_module_init(struct MLRectDim screen_dim)
{
    M.screen_dim = screen_dim;

    int atom = 16;

    for (int alpha = 0; alpha < 16; alpha++) {
        int v = 0;
        for (int r = 0; r < 32; r++) {
            M.alpha_r_lut[alpha << 5 | r] = (struct rgb565){(v >> 4) << 11};
            v += atom;
        }
        v = 0;
        for (int g = 0; g < 64; g++) {
            M.alpha_g_lut[alpha << 6 | g] = (struct rgb565){(v >> 4) << 5};
            v += atom;
        }
        v = 0;
        for (int b = 0; b < 32; b++) {
            M.alpha_b_lut[alpha << 5 | b] = (struct rgb565){(v >> 4)};
            v += atom;
        }
        atom--;
    }
}

bool
clip_gfx_frame(struct Clip* clip, const struct GfxFrame* gf, int x, int y)
{
    x -= gf->x_off;
    y -= gf->y_off;

    int x2 = x + gf->width - 1;
    int y2 = y + gf->height - 1;

    if (x >= M.screen_dim.w || x2 < 0 || y >= M.screen_dim.h || y2 < 0) {
        return false;
    }

    if (x < 0) {
        clip->sx_off = -x;
        x = 0;
    } else {
        clip->sx_off = 0;
    }
    if (x2 >= M.screen_dim.w) {
        x2 = M.screen_dim.w - 1;
    }

    if (y < 0) {
        clip->sy_off = -y;
        y = 0;
    } else {
        clip->sy_off = 0;
    }
    if (y2 >= M.screen_dim.h) {
        y2 = M.screen_dim.h - 1;
    }

    clip->x = x;
    clip->y = y;
    clip->w = x2 - x + 1;
    clip->h = y2 - y + 1;

    return true;
}

// For frames with alpha channel.
static void
blit_clipped_gfx_frame_with_alpha(const struct MLGraphicsBuffer* dst, const struct Clip* clip,
                                  const struct GfxFrame* gf)
{
    const int src_stride = gf->width - clip->w;
    const int dst_stride = dst->dim.w - clip->w;

    const uint8_t* src_alpha_p = &gf->alpha[clip->sy_off * gf->width + clip->sx_off];
    const struct rgb565* src_p = &gf->graphics[clip->sy_off * gf->width + clip->sx_off];
    struct rgb565* dst_p = ml_graphics_buffer_xy(dst, clip->x, clip->y);

    for (int y = 0; y < clip->h; y++) {
        for (int x = 0; x < clip->w; x++) {
            const int src_alpha = *src_alpha_p++;
            if (src_alpha == 0) {
                // Opaque
                *dst_p++ = *src_p++;
            } else {
                if (src_alpha == 16) {
                    // Transparent
                    src_p++;
                    dst_p++;
                } else {
                    const int dst_alpha = 16 - src_alpha;
                    const struct rgb src_color = unpack_rgb565(*src_p++);
                    const struct rgb dst_color = unpack_rgb565(*dst_p);

                    /* Historical note:
                     *
                     * Assembler? GCC produces ugly shit even on ARM
                     * wich have realy nice shift capabilities:(
                     *
                     * Use 2 MB table? 65536 * 2 * 16.
                     */
                    const struct rgb565 src_color2 = (struct rgb565) {
                        M.alpha_r_lut[(src_alpha << 5) | src_color.r].v |
                        M.alpha_g_lut[(src_alpha << 6) | src_color.g].v |
                        M.alpha_b_lut[(src_alpha << 5) | src_color.b].v
                    };

                    const struct rgb565 dst_color2 = (struct rgb565) {
                        M.alpha_r_lut[(dst_alpha << 5) | dst_color.r].v |
                        M.alpha_g_lut[(dst_alpha << 6) | dst_color.g].v |
                        M.alpha_b_lut[(dst_alpha << 5) | dst_color.b].v
                    };

                    *dst_p++ = add_rgb565(src_color2, dst_color2);
                }
            }
        }

        src_alpha_p += src_stride;
        src_p += src_stride;
        dst_p += dst_stride;
    }
}

// For frames without alpha channel.
static void
blit_clipped_gfx_frame_without_alpha(const struct MLGraphicsBuffer* dst, const struct Clip* clip, const struct GfxFrame* gf)
{
    const int src_stride = gf->width - clip->w;
    const int dst_stride = dst->dim.w - clip->w;

    const struct rgb565* src_p = &gf->graphics[clip->sy_off * gf->width + clip->sx_off];
    struct rgb565* dst_p = ml_graphics_buffer_xy(dst, clip->x, clip->y);

    for (int y = 0; y < clip->h; y++) {
        for (int x = 0; x < clip->w; x++) {
            const struct rgb565 color = *src_p++;
            if (color.v) {
                *dst_p = color;
            }
            dst_p++;
        }
        src_p += src_stride;
        dst_p += dst_stride;
    }
}

void
blit_clipped_gfx_frame(const struct MLGraphicsBuffer* dst, const struct Clip* clip, const struct GfxFrame* gf)
{
    if (gf->alpha) {
        blit_clipped_gfx_frame_with_alpha(dst, clip, gf);
    } else {
        blit_clipped_gfx_frame_without_alpha(dst, clip, gf);
    }
}

void
blit_clipped_colour_box(const struct MLGraphicsBuffer* dst, const struct Clip* clip, struct rgb565 color)
{
    const int dst_stride = dst->dim.w - clip->w;
    struct rgb565* dst_p = ml_graphics_buffer_xy(dst, clip->x, clip->y);

    for (int y = 0; y < clip->h; y++) {
        for (int x = 0; x < clip->w; x++) {
            *dst_p++ = color;
        }
        dst_p += dst_stride;
    }
}

void
blit_clipped_gfx_box(const struct MLGraphicsBuffer* dst, const struct Clip* clip, const struct rgb565* src)
{
    const int dst_stride = dst->dim.w - clip->w;
    struct rgb565* dst_p = ml_graphics_buffer_xy(dst, clip->x, clip->y);

    for (int y = 0; y < clip->h; y++) {
        for (int x = 0; x < clip->w; x++) {
            *dst_p++ = *src++;
        }
        dst_p += dst_stride;
    }
}
