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

static struct {
    struct MLRectDim screen_dim;
    struct Ufo ufo;
    struct Sprite score_sprite;
    int64_t score_until_ms;
    int64_t mystery_appear_ms;
} mystery_module;
#define M mystery_module

// Runlevel function.
static int
rl_play1_play2(struct RunLevelFunc* r)
{
    (void)r;
    return (player_is_alive() && (M.ufo.collision || M.score_until_ms > 0)) ? 0 : 1;
}

// Runlevel function.
static int
rl_play2_title0(struct RunLevelFunc* r)
{
    (void)r;
    return (M.ufo.collision || M.score_until_ms > 0) ? 0 : 1;
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

    M.score_sprite.x = M.ufo.sprite.x;
    M.score_sprite.y = MysteryYCoordinate;
    M.score_sprite.frame = score_frame;

    g_score += 50 + (score_frame * 50);
    M.score_until_ms = ml_time_milliseconds() + ScoreVisibleMs;

    sfx_mystery_explode();
    M.ufo.collision = 0;
    M.mystery_appear_ms = -1;

    return 1;
}

void
mystery_module_init(struct MLRectDim screen_dim)
{
    static struct RunLevelFunc rl0, rl1;

    M.screen_dim = screen_dim;

    M.score_until_ms = -1;
    M.mystery_appear_ms = -1;

    sprite_init(&M.score_sprite, gfx_object_find("score"), 0, 0, 0, true);

    runlevel_register_func(&rl0, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&rl1, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);
}

void
mystery_draw(const struct MLGraphicsBuffer* dst)
{
    if (M.ufo.collision) {
        sprite_draw(dst, &M.ufo.sprite);
    }
    if (M.score_until_ms > 0) {
        sprite_draw(dst, &M.score_sprite);
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

    const int64_t score_remain_ms = M.score_until_ms - ml_time_milliseconds();
    if (score_remain_ms >= 0) {
        const int64_t score_elapsed_ms = ScoreVisibleMs - score_remain_ms;
        M.score_sprite.y = MysteryYCoordinate - (score_elapsed_ms / 125);
    } else {
        M.score_until_ms = -1;
    }

    if (M.ufo.collision) {
        if (M.ufo.sprite.x == x_stop) {
            synth_channel_kill(CH_UFO_MOVE);
            collision_destroy(M.ufo.collision);
            M.ufo.collision = 0;
            M.mystery_appear_ms = -1;
        } else {
            ufo_anim(&M.ufo);
            M.ufo.sprite.x += x_vector;
            collision_update_from_sprite(M.ufo.collision, &M.ufo.sprite);
        }
    } else {
        if (g_runlevel      == RUNLEVEL_PLAY1 &&
            g_next_runlevel == RUNLEVEL_PLAY1) {
            if (M.mystery_appear_ms < 0) {
                M.mystery_appear_ms = next_mystery_appear_time_ms();
            } else if (M.mystery_appear_ms < ml_time_milliseconds()) {
                if (!f_direction) {
                    x_start = -20;
                    x_stop = M.screen_dim.w + 20;
                    x_vector = 2;
                } else {
                    x_start = M.screen_dim.w + 20;
                    x_stop = -20;
                    x_vector = -2;
                }
                ufo_init(&M.ufo, x_start, MysteryYCoordinate, true);
                M.ufo.collision = collision_create(0, 0, GID_MYSTERY, collision_cb);
                collision_update_from_sprite(M.ufo.collision, &M.ufo.sprite);
                sfx_mystery_move();
                f_direction ^= 1;
            }
        }
    }
}
