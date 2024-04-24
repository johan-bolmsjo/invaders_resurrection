#include "mystery.h"

#include <stdlib.h>
#include <inttypes.h>

#include "gids.h"
#include "libmedia/libmedia.h"
#include "libsynth/libsynth.h"
#include "player.h"
#include "runlevel.h"
#include "sfx.h"
#include "status.h"
#include "ufo.h"

#define MYSTERY_TIMER (30.0 * MLDisplayFreq) /* Mystery max delay in VBLs */
#define SCORE_TIMER   (3 * MLDisplayFreq)    /* Time to display the score */
#define MYSTERY_Y     32                /* Mystery Y coordinate */

static struct Ufo ufo = {0};
static int score_timer = 0;
static struct Sprite score_sprite = {0};

// Runlevel function.
static int
rl_play1_play2(struct RunLevelFunc* r)
{
    (void)r;
    return (player_is_alive() && (ufo.collision || score_timer)) ? 0 : 1;
}

// Runlevel function.
static int
rl_play2_title0(struct RunLevelFunc* r)
{
    (void)r;
    return (ufo.collision || score_timer) ? 0 : 1;
}

static int
collision_cb(struct Collision* a, struct Collision* b)
{
    (void)a;
    (void)b;

    if (b->gid != GID_PLAYER_SHOT) {
        return 0;
    }

    int score = (int)(3.0 * random() / (RAND_MAX + 1.0));

    score_sprite.x = ufo.sprite.x;
    score_sprite.y = ufo.sprite.y;
    score_sprite.frame = score;

    g_score += 50 + (score * 50);
    score_timer = SCORE_TIMER;

    sfx_mystery_explode();
    ufo.collision = 0;

    return 1;
}

void
mystery_module_init(void)
{
    static struct RunLevelFunc rl0, rl1;

    sprite_init(&score_sprite, gfx_object_find("score"), 0, 0, 0, 1);

    runlevel_register_func(&rl0, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&rl1, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);
}

void
mystery_draw(const struct MLGraphicsBuffer* dst)
{
    if (ufo.collision) {
        sprite_draw(dst, &ufo.sprite);
    }

    if (score_timer) {
        sprite_draw(dst, &score_sprite);
    }
}

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

    if (ufo.collision) {
        if (ufo.sprite.x == x_stop) {
            synth_channel_kill(CH_UFO_MOVE);
            collision_destroy(ufo.collision);
            ufo.collision = 0;
        } else {
            ufo_anim(&ufo);
            ufo.sprite.x += x_vector;
            collision_update_from_sprite(ufo.collision, &ufo.sprite);
        }
    } else {
        if (g_runlevel == RUNLEVEL_PLAY1 &&
            g_next_runlevel == RUNLEVEL_PLAY1) {
            if (!timer) {
                if (!dir) {
                    x_start = -20;
                    x_stop = MLDisplayWidth + 20;
                    x_vector = 2;
                } else {
                    x_start = MLDisplayWidth + 20;
                    x_stop = -20;
                    x_vector = -2;
                }

                ufo_init(&ufo, x_start, MYSTERY_Y, 1);
                ufo.collision = collision_create(0, 0, GID_MYSTERY, collision_cb);
                collision_update_from_sprite(ufo.collision, &ufo.sprite);
                sfx_mystery_move();

                timer = (int)(MYSTERY_TIMER * random() / (RAND_MAX + 1.0));
                dir ^= 1;

            } else {
                timer--;
            }
        }
    }
}
