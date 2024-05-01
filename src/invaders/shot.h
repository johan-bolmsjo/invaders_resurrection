#pragma once
/// \file shot.h
///
/// Player shots and collision particles.
///

#include <inttypes.h>

#include "collision.h"
#include "libmedia/libmedia.h"
#include "libutil/color.h"

struct Shot;

extern int g_shot_obj;

void shot_module_init(struct MLRectDim screen_dim);

/// Create shot.
struct Shot* shot_create(int x, int y, int x_vector, int y_vector, struct rgb565 color,
                         int fatal, int gid, void (*cb)(void));

/// Draw all shots to screen.
void shot_draw(const struct MLGraphicsBuffer* dst);

/// Update all shots.
void shot_update(void);
