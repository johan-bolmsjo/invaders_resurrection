/* The mystery is the UFO that goes forth and back in the upper
 * region of the screen.
 */

#include "mystery.h"

#include <stdlib.h>
#include <inttypes.h>

#include "gids.h"
#include "libsynth/libsynth.h"
#include "player.h"
#include "runlevel.h"
#include "sfx.h"
#include "status.h"
#include "ufo.h"

#define MYSTERY_TIMER (30.0 * DG_VFREQ) /* Mystery max delay in VBLs */
#define SCORE_TIMER   (3 * DG_VFREQ)    /* Time to display the score */
#define MYSTERY_Y     32                /* Mystery Y coordinate */

static Ufo ufo = {0};
static int score_timer = 0;
static Sprite score_sprite = {0};

/* Runlevel function.
 */

static int
rl_play1_play2(RunLevelFunc* r)
{
    (void)r;

    if (g_player_alive && (ufo.c || score_timer))
        return 0;

    return 1;
}

/* Runlevel function.
 */

static int
rl_play2_title0(RunLevelFunc* r)
{
    (void)r;

    if (ufo.c || score_timer)
        return 0;

    return 1;
}

/* Collision callback.
 */

static int
c_cb(Collision* a, Collision* b)
{
    (void)a;
    (void)b;

    int score;

    if (b->gid != GID_PLAYER_SHOT)
        return 0;

    score = (int)(3.0 * random() / (RAND_MAX + 1.0));

    score_sprite.x = ufo.s.x;
    score_sprite.y = ufo.s.y;
    score_sprite.frame = score;

    g_score += 50 + (score * 50);
    score_timer = SCORE_TIMER;

    sfx_mystery_explode();
    ufo.c = 0;

    return 1;
}

/* Create tables etc.
 */

void
mystery_tables(void)
{
    static RunLevelFunc rl0, rl1;

    sprite_init(&score_sprite, gfx_object_find("score"), 0, 0, 0, 1);

    runlevel_register_func(&rl0, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&rl1, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);
}

/* Hide mystery.
 */

void
mystery_hide(DG* dg)
{
    sprite_hide(dg, &ufo.s);
    sprite_hide(dg, &score_sprite);
}

/* Show mystery.
 */

void
mystery_show(DG* dg)
{
    if (ufo.c)
        sprite_show(dg, &ufo.s);

    if (score_timer)
        sprite_show(dg, &score_sprite);
}

/* Updates and creates "mysteries" during game play.
 */

void
mystery_update(void)
{
    static int init = 0, dir = 0, x_start, x_stop, x_vector, timer;

    if (!init) {
        timer = (int)(MYSTERY_TIMER * random() / (RAND_MAX + 1.0));
        init = 1;
    }

    if (score_timer) {
        if (!(score_timer & 7))
            score_sprite.y--;
        score_timer--;
    }

    if (ufo.c) {
        if (ufo.s.x == x_stop) {
            synth_channel_kill(CH_UFO_MOVE);
            collision_destroy(ufo.c);
            ufo.c = 0;
        } else {
            ufo_anim(&ufo);
            ufo.s.x += x_vector;
            collision_update_from_sprite(ufo.c, &ufo.s);
        }
    } else {
        if (g_runlevel == RUNLEVEL_PLAY1 &&
            g_next_runlevel == RUNLEVEL_PLAY1) {
            if (!timer) {
                if (!dir) {
                    x_start = -20;
                    x_stop = DG_XRES + 20;
                    x_vector = 2;
                } else {
                    x_start = DG_XRES + 20;
                    x_stop = -20;
                    x_vector = -2;
                }

                ufo_init(&ufo, x_start, MYSTERY_Y, 1);
                ufo.c = collision_create(0, 0, GID_MYSTERY, c_cb);
                collision_update_from_sprite(ufo.c, &ufo.s);
                sfx_mystery_move();

                timer = (int)(MYSTERY_TIMER * random() / (RAND_MAX + 1.0));
                dir ^= 1;

            } else {
                timer--;
            }
        }
    }
}
