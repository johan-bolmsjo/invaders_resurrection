#include "ufo.h"

#include <stdio.h>
#include <math.h>
#include <inttypes.h>

static int count[15];
static struct GfxObject* ufo_object;

void
ufo_module_init(void)
{
    int i;
    double step = 1.5708 / 7, pos = -1.5708;

    for (i = 0; i < 15; i++) {
        count[i] = floor(1 + 2 * sin(fabs(pos)) + 0.5);
        pos += step;
    }

    count[0] = 6;
    count[14] = 6;

    ufo_object = gfx_object_find("ufo");
}

void
ufo_init(struct Ufo* ufo, int x, int y, bool show)
{
    ufo->frame_count = 0;
    ufo->direction   = 1;

    sprite_init(&ufo->sprite, ufo_object, 7, x, y, show);
}

void
ufo_anim(struct Ufo* ufo)
{
    int frame = ufo->sprite.frame;

    ufo->frame_count++;
    if (ufo->frame_count > count[frame]) {
        if (frame == 0 || frame == 14) {
            ufo->direction = -ufo->direction;
        }
        frame += ufo->direction;
        ufo->frame_count = 0;
    }

    ufo->sprite.frame = frame;
}
