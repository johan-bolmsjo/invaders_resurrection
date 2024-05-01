#pragma once
/// \file sfx.h
///
/// All game sound effects.
///

// Channels used for the differnt sound effects.
// The synth have four channels.
//
// TODO(jb): make enum of constants

enum {
    ChannelPlayerShot = 0,
    ChannelPlayerDie  = 0,
    ChannelExtraLife  = 0,
    ChannelBomberMove = 1,
    ChannelBomberDie  = 2,
    ChannelUfoMove    = 3,
    ChannelUfoDie     = 3,
};

/// Extra life sound.
void sfx_extra_life(void);

/// Player shot sound.
void sfx_player_shot(void);

/// Player explodes sound.
void sfx_player_explode(void);

/// Bomber movement sound.
void sfx_bomber_move(void);

/// Bomber explodes sound.
void sfx_bomber_explode(void);

/// Mystery movement sound.
void sfx_mystery_move(void);

/// Mystery explodes sound.
void sfx_mystery_explode(void);
