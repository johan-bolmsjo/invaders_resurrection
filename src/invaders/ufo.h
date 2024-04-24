#pragma once

#include "collision.h"
#include "sprite.h"

struct Ufo {
    int frame_count;
    int direction;
    struct Sprite sprite;
    struct Collision* collision;
};

void ufo_module_init(void);
void ufo_init(struct Ufo* ufo, int x, int y, bool show);
void ufo_anim(struct Ufo* ufo);
