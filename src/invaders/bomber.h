#pragma once

#include "sprite.h"
#include "collision.h"

typedef struct _Bomber {
    int        count;
    int        x, y;            // Used in armada.c
    Sprite     s;
    Collision* c;
} Bomber;

/// Initialise bomber.
void bomber_init(Bomber* b);

/// Animate bomber.
void bomber_anim(Bomber* b);
