#include "ufo.h"

#include <stdio.h>
#include <math.h>
#include <inttypes.h>

static int count[15];
static GfxObject* ufo_object;

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
ufo_init(Ufo* u, int x, int y, int vis)
{
    u->count = 0;
    u->dir = 1;

    sprite_init(&u->s, ufo_object, 7, x, y, vis);
}

void
ufo_anim(Ufo* u)
{
    int frame = u->s.frame;

    u->count++;
    if (u->count > count[frame]) {
        if (frame == 0 || frame == 14)
            u->dir = -u->dir;
        frame += u->dir;
        u->count = 0;
    }

    u->s.frame = frame;
}
