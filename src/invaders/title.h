#pragma once
/// \file title.h
///
/// Title screen management.
///

#include "libmedia/libmedia.h"

/// Initialize module.
void title_module_init(void);

/// Draw title objects.
void title_show(const DG* dg);

/// Clear title objects.
void title_hide(const DG* dg);

/// Manages changes.
/// Returns 1 if game should terminate.
int title_update(const DG* dg, struct MLInput* input);
