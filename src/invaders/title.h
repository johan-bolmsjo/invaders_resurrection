#pragma once
/// \file title.h
///
/// Title screen management.
///

#include "libmedia/libmedia.h"

enum GameRunState {
    GameContinue,
    GameExit,
};

/// Initialize module.
void title_module_init(void);

/// Draw title objects.
void title_draw(const DG* dg, const struct MLGraphicsBuffer* buf);

/// Update title screen.
enum GameRunState title_update(struct MLInput* input);
