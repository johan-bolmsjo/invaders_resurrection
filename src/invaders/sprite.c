#include "sprite.h"
#include "libutil/color.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void
sprite_init(Sprite* sprite, struct GfxObject* go, int frame, int x, int y, bool show)
{
    sprite->show = show;
    sprite->x = x;
    sprite->y = y;
    sprite->frame = frame;
    sprite->go = go;
    sprite->blit[0] = 0;
    sprite->blit[1] = 0;
}

void
sprite_hide(const DG* dg, Sprite* sprite)
{
    if (sprite->blit[dg->hid]) {
        // TODO(jb): Compatibility; remove
        struct MLGraphicsBuffer draw_buf = ml_graphics_buffer_of_dg(dg);
        blit_clipped_colour_box(&draw_buf, &sprite->clip[dg->hid], pack_rgb565(rgb565_color_black()));
        sprite->blit[dg->hid] = 0;
    }
}

void
sprite_draw(const DG* dg, Sprite* sprite)
{
    struct Clip*     clip = &sprite->clip[dg->hid];
    struct GfxFrame* gf   = sprite->go->fpp[sprite->frame];

    if (sprite->show) {
        if (clip_gfx_frame(clip, gf, sprite->x, sprite->y)) {
            // TODO(jb): Compatibility; remove
            struct MLGraphicsBuffer draw_buf = ml_graphics_buffer_of_dg(dg);
            blit_clipped_gfx_frame(&draw_buf, clip, gf);
            sprite->blit[dg->hid] = 1;
        }
    }
}
