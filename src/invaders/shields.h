#pragma once

#include <stdbool.h>

#include "dg.h"

// Create shield data and other initialisations.
void shield_tables(void);

// Create new shields.
void shields_new(void);

// Delete shields.
void shields_del(void);

// Makes a hole in shield and returns true if it was hit (shots may tunnel through).
// x and y are screen coordinates, y_vec is the direction of the hit (1 moving downwards, or -1 upwards).
// shield_id is the registered collision ID for the shield.
bool shields_hit(int x, int y, int y_vec, int shield_id);

// Copy shields to the screen.
void shields_show(DG* dg);

// Delete shields from the screen.
void shields_hide(DG* dg);
