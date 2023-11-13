#pragma once
/// \file mystery.h
///
/// The mystery is the UFO that goes forth and back in the upper region
/// of the screen.

#include "libmedia/libmedia.h"

/// Initialize module.
void mystery_module_init(void);

/// Hide mystery.
void mystery_hide(const DG* dg);

/// Show mystery.
void mystery_show(const DG* dg);

/// Updates and creates "mysteries" during game play.
void mystery_update(void);
