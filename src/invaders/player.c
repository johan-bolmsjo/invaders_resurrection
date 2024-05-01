#include "player.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gids.h"
#include "libutil/array.h"
#include "libutil/color.h"
#include "missiles.h"
#include "runlevel.h"
#include "sfx.h"
#include "shot.h"
#include "status.h"

struct Player {
    int               steer_count;
    struct Sprite     sprite;
    struct Collision* collision;
};

static struct {
    struct MLRectDim screen_dim;
    struct MLInput* input_ref;

    int x_vector[15];
    int y_vector[16];
    int x_vector_pos;
    int y_vector_pos;

    int               count[15];
    int               live_shots;
    struct Player     player; // Player information
    struct GfxObject* player_obj;

    bool is_alive;
} player_module;
#define M player_module

#define MinPlayerPositionX (60)
#define MaxPlayerPositionX (M.screen_dim.w - MinPlayerPositionX)


// Make sure that no shots are alive before
// changing from runlevel play1 to play2.
static int
rl_play1_play2(struct RunLevelFunc* r)
{
    (void)r;
    return !M.live_shots ? 1 : 0;
}

// Make sure that no missiles are alive before
// changing from runlevel play2 to play0.
static int
rl_play2_play0(struct RunLevelFunc* r)
{
    (void)r;
    return !g_missiles_alive ? 1 : 0;
}

// Make sure that no objects or shots are alive before
// changing from runlevel play2 to title0.
//
// Also clear joystick button flag.
static int
rl_play2_title0(struct RunLevelFunc* r)
{
    (void)r;

    static int pend = 0;

    if (!(g_collision_obj + g_shot_obj)) {
        pend++;
    }

    if (pend == 2) {
        pend = 0;
        return 1;
    }

    M.input_ref->press_button_fire = false;

    return 0;
}

void
player_module_init(struct MLRectDim screen_dim, struct MLInput* input)
{
    M.screen_dim = screen_dim;
    M.input_ref = input;

    const double step = 1.5708 / 7;
    double pos = -1.5708;

    for (size_t i = 0; i < ARRAY_SIZE(M.count); i++) {
        M.count[i] = floor(2 * sin(fabs(pos)) + 0.5);
        pos += step;
    }

    for (size_t i = 0; i < ARRAY_SIZE(M.x_vector); i++) {
        M.x_vector[i] = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
    }

    for (size_t i = 0; i < ARRAY_SIZE(M.y_vector); i++) {
        M.y_vector[i] = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
    }

    M.player_obj = gfx_object_find("player");

    static struct RunLevelFunc play1_play2;
    static struct RunLevelFunc play2_play0;
    static struct RunLevelFunc play2_title0;

    runlevel_register_func(&play1_play2, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&play2_play0, RUNLEVEL_PLAY2, RUNLEVEL_PLAY0,
                           rl_play2_play0);

    runlevel_register_func(&play2_title0, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);
}

// Animate player
static void
animate(struct Player* p, struct MLInput* input)
{
    int frame = p->sprite.frame;

    p->steer_count++;
    if (p->steer_count > M.count[frame]) {
        if (input->x_axis != 0) {
            frame += input->x_axis;
            if (frame < 0) {
                frame = 0;
            }
            if (frame > 14) {
                frame = 14;
            }
        } else {
            if (frame < 7) {
                frame++;
            }
            if (frame > 7) {
                frame--;
            }
        }
        p->steer_count = 0;
    }

    p->sprite.frame = frame;
}

// Move player
static void
move(struct Player* p)
{
    int frame = p->sprite.frame;

    if (frame < 7) {
        p->sprite.x -= M.count[frame];
        if (p->sprite.x < MinPlayerPositionX) {
            p->sprite.x = MinPlayerPositionX;
        }
    }
    if (frame > 7) {
        p->sprite.x += M.count[frame];
        if (p->sprite.x > MaxPlayerPositionX) {
            p->sprite.x = MaxPlayerPositionX;
        }
    }

    collision_update_from_sprite(p->collision, &p->sprite);
}

// Callback function to keep track of how many shots that's in the air.
static void
shot_cb(void)
{
    M.live_shots--;
}

// Kill pilot.
static void
die(void)
{
    const int x = M.player.sprite.x;
    const int y = M.player.sprite.y;

    for (int i = 0; i < 8; i++) {
        const int xv = M.x_vector[M.x_vector_pos++];
        if (M.x_vector_pos == ARRAY_SIZE(M.x_vector)) {
            M.x_vector_pos = 0;
        }

        int yv;
        do {
            yv = M.y_vector[M.y_vector_pos++];
            if (M.y_vector_pos == ARRAY_SIZE(M.y_vector)) {
                M.y_vector_pos = 0;
            }
        } while ((abs(xv) + abs(yv)) < 3);

        shot_create(x + xv, y + yv, xv, yv, pack_rgb565((struct rgb){.g =  31, .b = 31}), 0, 0, 0);
    }

    if (g_runlevel == RUNLEVEL_PLAY1) {
        g_next_runlevel = RUNLEVEL_PLAY2;
    }

    g_pilots--;
    M.is_alive = false;

    sfx_player_explode();
}

void
player_kill(void)
{
    if (M.player.collision) {
        die();
        collision_destroy(M.player.collision);
        M.player.collision = 0;
    }
}

static int
collision_cb(struct Collision* a, struct Collision* b)
{
    (void)a;
    (void)b;

    die();
    M.player.collision = 0;
    return 1;
}

void
player_draw(const struct MLGraphicsBuffer* dst)
{
    if (M.player.collision) {
        sprite_draw(dst, &M.player.sprite);
    }
}

// New player
static void
player_new(void)
{
    if (!M.player.collision) {
        sprite_init(&M.player.sprite, M.player_obj, 7, M.screen_dim.w / 2, M.screen_dim.h - 16, true);
        M.player.steer_count = 0;
        M.player.collision = collision_create(0, 0, GID_PLAYER, collision_cb);
        collision_update_from_sprite(M.player.collision, &M.player.sprite);
        M.is_alive = true;
    }
}

// Give up and return to the title screen.
static void
suicide(void)
{
    if (M.player.collision) {
        collision_destroy(M.player.collision);
        M.player.collision = 0;
        g_pilots = 0;
        M.is_alive = false;
    }
}

void
player_update(struct MLInput* input)
{
    if (g_runlevel >= RUNLEVEL_PLAY0 &&
        g_runlevel <= RUNLEVEL_PLAY2) {

        if (input->press_button_back) {
            input->press_button_back = false;
            suicide();
        }

        if (g_pilots) {
            if (g_runlevel == RUNLEVEL_PLAY0 && !M.player.collision) {
                player_new();
            }

            if (M.player.collision) {
                animate(&M.player, input);
                move(&M.player);

                if (input->press_button_fire) {
                    input->press_button_fire = false;

                    const bool quickshot = false; // For testing
                    if ((!M.live_shots || quickshot) &&
                        g_runlevel == RUNLEVEL_PLAY1) {
                        if (shot_create(M.player.sprite.x - 1, M.player.sprite.y - 16,
                                        0, -8, pack_rgb565(rgb565_color_white()), 1, GID_PLAYER_SHOT, shot_cb)) {
                            M.live_shots++;
                        }
                        sfx_player_shot();
                    }
                }
            }
        } else {
            if (g_runlevel == RUNLEVEL_PLAY0 ||
                g_runlevel == RUNLEVEL_PLAY1) {
                g_next_runlevel = RUNLEVEL_PLAY2;
            }

            if (g_runlevel == RUNLEVEL_PLAY2) {
                g_next_runlevel = RUNLEVEL_TITLE0;
            }
        }
    }
}

bool
player_is_alive(void)
{
    return M.is_alive;
}
