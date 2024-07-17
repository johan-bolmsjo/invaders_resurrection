#pragma once
/// \file mystery.h
///
/// The mystery is the UFO that goes forth and back in the upper region
/// of the screen.

#include "libmedia/libmedia.h"
#include "libutil/prng.h"

/// Initialize module.
void mystery_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state);

/// Draw mystery to screen.
void mystery_draw(const struct MLGraphicsBuffer* dst);

/// Updates and creates "mysteries" during game play.
void mystery_update(void);
