#pragma once

#include "dg.h"
#include "joy.h"

void title_tables(void);
void title_show(DG* dg);
void title_hide(DG* dg);
int  title_update(DG* dg, Joy* j, int key_q);
