#include "bomber.h"

void
bomber_init(Bomber* b)
{
    b->count = 0;
}

void
bomber_anim(Bomber* b)
{
    int frame = b->s.frame;

    b->count++;
    if (b->count > 1) {
        b->count = 0;
        frame++;
        if (frame >= b->s.go->frames)
            frame = 0;
    }

    b->s.frame = frame;
}
