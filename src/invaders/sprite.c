#include "sprite.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void
sprite_init(Sprite* sprite, GfxObject* go, int frame, int x, int y, int vis)
{
    sprite->vis = vis;
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
        blit_clipped_colour_box(dg, &sprite->clip[dg->hid], 0);
        sprite->blit[dg->hid] = 0;
    }
}

void
sprite_show(const DG* dg, Sprite* sprite)
{
    Clip* clip = &sprite->clip[dg->hid];
    GfxFrame* gf = sprite->go->fpp[sprite->frame];

    if (sprite->vis) {
        if (!clip_gfx_frame(clip, gf, sprite->x, sprite->y)) {
            blit_clipped_gfx_frame(dg, clip, gf);
            sprite->blit[dg->hid] = 1;
        }
    }
}
