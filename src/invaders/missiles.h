#pragma once
/// \file missiles.h
///
/// Invaders (alien bombers) missiles.
///

#include "collision.h"
#include "libmedia/libmedia.h"
#include "sprite.h"

typedef struct _Missile {
    Sprite     s;
    Collision* c;
} Missile;

extern int g_missiles_alive;

/// Initialize module.
void missiles_module_init(void);

/// Draw missiles on screen.
void missiles_show(const DG* dg);

/// Remove missiles from screen.
void missiles_hide(const DG* dg);

/// Updates old missiles and creates new ones.
void missiles_update(void);
