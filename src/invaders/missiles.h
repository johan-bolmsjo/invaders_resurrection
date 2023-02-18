#pragma once

typedef struct _Missile {
    Sprite s;
    Collision* c;
} Missile;

extern int g_missiles_alive;

void missiles_tables();
void missiles_show(DG* dg);
void missiles_hide(DG* dg);
void missiles_update();
