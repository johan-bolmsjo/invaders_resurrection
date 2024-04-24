#include "sprite.h"
#include "libutil/color.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void
sprite_init(struct Sprite* sprite, struct GfxObject* go, int frame, int x, int y, bool show)
{
    sprite->show = show;
    sprite->x = x;
    sprite->y = y;
    sprite->frame = frame;
    sprite->gfx_obj = go;
}

void
sprite_draw(const struct MLGraphicsBuffer* dst, const struct Sprite* sprite)
{
    if (sprite->show) {
        struct Clip clip;
        const struct GfxFrame* gf = sprite->gfx_obj->fpp[sprite->frame];

        if (clip_gfx_frame(&clip, gf, sprite->x, sprite->y)) {
            blit_clipped_gfx_frame(dst, &clip, gf);
        }
    }
}
