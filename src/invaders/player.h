#pragma once

#include "collision.h"
#include "libmedia/libmedia.h"
#include "sprite.h"

/// Initialize module.
void player_module_init(struct MLRectDim screen_dim, struct MLInput* input);

/// Kill pilot and destroy its collision object.
void player_kill(void);

/// Draw player.
void player_draw(const struct MLGraphicsBuffer* dst);

/// Update player based on input.
void player_update(struct MLInput* input);

/// Report whether player is currently alive.
bool player_is_alive(void);
