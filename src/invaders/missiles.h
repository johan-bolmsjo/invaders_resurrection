#pragma once
/// \file missiles.h
///
/// Invaders (alien bombers) missiles.
///

#include "collision.h"
#include "libmedia/libmedia.h"
#include "libutil/prng.h"
#include "sprite.h"

struct Missile {
    struct Sprite     sprite;
    struct Collision* collision;
};

extern int g_missiles_alive;

/// Initialize module.
void missiles_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state);

/// Draw missiles to screen.
void missiles_draw(const struct MLGraphicsBuffer* dst);

/// Updates old missiles and creates new ones.
void missiles_update(void);
