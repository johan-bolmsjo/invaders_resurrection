#pragma once
/// \file stars.h
///
/// Star field.. Whohaa.
///

#include "libmedia/libmedia.h"

/// Initialize stars module.
void stars_module_init(struct MLRectDim screen_dim);

/// Plot stars.
void stars_draw(const struct MLGraphicsBuffer* dst);
