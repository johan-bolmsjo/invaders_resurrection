#pragma once

#include "dg.h"

void shield_tables(void);
void shields_new(void);
void shields_del(void);
int  shields_hit(int x_pos, int y_pos, int y_vec, int shield);
void shields_show(DG* dg);
void shields_hide(DG* dg);
