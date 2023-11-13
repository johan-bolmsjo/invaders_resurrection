#pragma once

#include "collision.h"
#include "libmedia/libmedia.h"
#include "sprite.h"

typedef struct _Player {
    int        count;
    Sprite     s;
    Collision* c;
} Player;

/// Initialize module.
void player_module_init(struct MLInput* input);

/// Kill pilot and destroy its collision object.
void player_kill(void);

/// Hide player.
void player_hide(const DG* dg);

/// Show player.
void player_show(const DG* dg);

/// Update player based on input.
void player_update(struct MLInput* input);

/// Report whether player is currently alive.
bool player_is_alive(void);
