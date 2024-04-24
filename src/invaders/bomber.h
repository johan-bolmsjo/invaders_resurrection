#pragma once

#include "sprite.h"
#include "collision.h"

struct Bomber {
    int               count;
    int               x, y;     // Used in armada.c
    struct Sprite     sprite;
    struct Collision* collision;
};

/// Initialise bomber.
void bomber_init(struct Bomber* b);

/// Animate bomber.
void bomber_anim(struct Bomber* b);
