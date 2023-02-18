#pragma once

#include "collision.h"
#include "dg.h"
#include "sprite.h"

typedef struct _Missile {
    Sprite s;
    Collision* c;
} Missile;

extern int g_missiles_alive;

void missiles_tables(void);
void missiles_show(DG* dg);
void missiles_hide(DG* dg);
void missiles_update(void);
