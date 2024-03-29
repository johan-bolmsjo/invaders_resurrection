#pragma once

#include "collision.h"
#include "sprite.h"

typedef struct _Ufo {
    int count;
    int dir;
    Sprite s;
    Collision* c;
} Ufo;

void ufo_module_init(void);
void ufo_init(Ufo* u, int x, int y, int vis);
void ufo_anim(Ufo* u);
