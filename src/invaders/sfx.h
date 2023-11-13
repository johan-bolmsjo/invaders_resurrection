#pragma once
/// \file sfx.h
///
/// All game sound effects.
///

// Channels used for the differnt sound effects.
// The synth have four channels.
//
// TODO(jb): make enum of constants

#define CH_PLAYER_SHOT 0
#define CH_PLAYER_DIE  0

#define CH_EXTRA_LIFE 0

#define CH_BOMBER_MOVE 1
#define CH_BOMBER_DIE  2

#define CH_UFO_MOVE 3
#define CH_UFO_DIE  3

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
