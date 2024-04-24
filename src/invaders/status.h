#pragma once
/// \file status.h
///
/// Game status bar.
///

#include "libmedia/libmedia.h"

extern unsigned int g_pilots;
extern unsigned int g_score;
extern unsigned int g_hi_score;

/// Reset status.
void status_reset(void);

/// Update and draw status list.
void status_draw(const struct MLGraphicsBuffer* dst);
