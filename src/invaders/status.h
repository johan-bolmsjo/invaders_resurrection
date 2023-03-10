#pragma once

#include "dg.h"

extern unsigned int g_pilots;
extern unsigned int g_score;
extern unsigned int g_hi_score;

void status_reset(void);
void status_hide(DG* dg);
void status_show(DG* dg);
