#include "bomber.h"

void
bomber_init(struct Bomber* b)
{
    b->count = 0;
}

void
bomber_anim(struct Bomber* b)
{
    int frame = b->sprite.frame;

    b->count++;
    if (b->count > 1) {
        b->count = 0;
        frame++;
        if (frame >= b->sprite.gfx_obj->frames) {
            frame = 0;
        }
    }

    b->sprite.frame = frame;
}
