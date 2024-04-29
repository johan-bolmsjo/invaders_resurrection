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

enum {
    MysteryMaxDelayMs  = 30000,
    MysteryMinDelayMs  = 3000,
    MysteryYCoordinate = 32,
    ScoreVisibleMs     = 3000,
};

static struct Ufo ufo = {0};
static struct Sprite score_sprite = {0};
static int64_t score_until_ms = -1;
static int64_t mystery_appear_ms = -1;

// Runlevel function.
static int
rl_play1_play2(struct RunLevelFunc* r)
{
    (void)r;
    return (player_is_alive() && (ufo.collision || score_until_ms > 0)) ? 0 : 1;
}

// Runlevel function.
static int
rl_play2_title0(struct RunLevelFunc* r)
{
    (void)r;
    return (ufo.collision || score_until_ms > 0) ? 0 : 1;
}

static int
collision_cb(struct Collision* a, struct Collision* b)
{
    (void)a;
    (void)b;

    if (b->gid != GID_PLAYER_SHOT) {
        return 0;
    }

    int score_frame = (int)(3.0 * random() / (RAND_MAX + 1.0));

    score_sprite.x = ufo.sprite.x;
    score_sprite.y = MysteryYCoordinate;
    score_sprite.frame = score_frame;

    g_score += 50 + (score_frame * 50);
    score_until_ms = ml_time_milliseconds() + ScoreVisibleMs;

    sfx_mystery_explode();
    ufo.collision = 0;
    mystery_appear_ms = -1;

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
    if (score_until_ms > 0) {
        sprite_draw(dst, &score_sprite);
    }
}

static int64_t
next_mystery_appear_time_ms(void)
{
    return ml_time_milliseconds() + (((int64_t)(MysteryMaxDelayMs - MysteryMinDelayMs) * random() + (RAND_MAX / 2)) / RAND_MAX + MysteryMinDelayMs);
}

void
mystery_update(void)
{
    static int f_direction = 0, x_start, x_stop, x_vector;

    const int64_t score_remain_ms = score_until_ms - ml_time_milliseconds();
    if (score_remain_ms >= 0) {
        const int64_t score_elapsed_ms = ScoreVisibleMs - score_remain_ms;
        score_sprite.y = MysteryYCoordinate - (score_elapsed_ms / 125);
    } else {
        score_until_ms = -1;
    }

    if (ufo.collision) {
        if (ufo.sprite.x == x_stop) {
            synth_channel_kill(CH_UFO_MOVE);
            collision_destroy(ufo.collision);
            ufo.collision = 0;
            mystery_appear_ms = -1;
        } else {
            ufo_anim(&ufo);
            ufo.sprite.x += x_vector;
            collision_update_from_sprite(ufo.collision, &ufo.sprite);
        }
    } else {
        if (g_runlevel      == RUNLEVEL_PLAY1 &&
            g_next_runlevel == RUNLEVEL_PLAY1) {
            if (mystery_appear_ms < 0) {
                mystery_appear_ms = next_mystery_appear_time_ms();
            } else if (mystery_appear_ms < ml_time_milliseconds()) {
                if (!f_direction) {
                    x_start = -20;
                    x_stop = MLDisplayWidth + 20;
                    x_vector = 2;
                } else {
                    x_start = MLDisplayWidth + 20;
                    x_stop = -20;
                    x_vector = -2;
                }
                ufo_init(&ufo, x_start, MysteryYCoordinate, true);
                ufo.collision = collision_create(0, 0, GID_MYSTERY, collision_cb);
                collision_update_from_sprite(ufo.collision, &ufo.sprite);
                sfx_mystery_move();
                f_direction ^= 1;
            }
        }
    }
}
