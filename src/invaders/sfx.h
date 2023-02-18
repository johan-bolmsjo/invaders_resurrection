/* Channels used for the differnt sound effects.
 * The synth have four channels.
 */

#define CH_PLAYER_SHOT 0
#define CH_PLAYER_DIE  0

#define CH_EXTRA_LIFE 0

#define CH_BOMBER_MOVE 1
#define CH_BOMBER_DIE  2

#define CH_UFO_MOVE 3
#define CH_UFO_DIE  3

void sfx_extra_life();
void sfx_player_shot();
void sfx_player_explode();
void sfx_bomber_move();
void sfx_bomber_explode();
void sfx_mystery_move();
void sfx_mystery_explode();
