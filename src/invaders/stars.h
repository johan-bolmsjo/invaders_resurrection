#pragma once
/// \file stars.h
///
/// Star field.. Whohaa.
///

#include "libmedia/libmedia.h"
#include "libutil/prng.h"

/// Initialize stars module.
void stars_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state);

/// Plot stars.
void stars_draw(const struct MLGraphicsBuffer* dst);
