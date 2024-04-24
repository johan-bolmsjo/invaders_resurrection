#include "player.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gids.h"
#include "libutil/color.h"
#include "missiles.h"
#include "runlevel.h"
#include "sfx.h"
#include "shot.h"
#include "status.h"

#define X_VECTORS 15
#define Y_VECTORS 16

#define X_MIN 60
#define X_MAX (MLDisplayWidth - X_MIN)

static int x_vector[X_VECTORS];
static int y_vector[Y_VECTORS];
static int x_vector_pos = 0;
static int y_vector_pos = 0;

struct Player {
    int               steer_count;
    struct Sprite     sprite;
    struct Collision* collision;
};

static int               count[15];
static int               live_shots = 0;
static struct Player     player; // Player information
static struct GfxObject* player_obj;

// Set in player_init(), used by runlevel func
static struct MLInput* input_ref;

static int is_alive = false;


// Make sure that no shots are alive before
// changing from runlevel play1 to play2.
static int
rl_play1_play2(struct RunLevelFunc* r)
{
    (void)r;
    return !live_shots ? 1 : 0;
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

    input_ref->press_button_a = false;

    return 0;
}

void
player_module_init(struct MLInput* input)
{
    double step = 1.5708 / 7, pos = -1.5708;
    static struct RunLevelFunc play1_play2;
    static struct RunLevelFunc play2_play0;
    static struct RunLevelFunc play2_title0;

    for (int i = 0; i < 15; i++) {
        count[i] = floor(2 * sin(fabs(pos)) + 0.5);
        pos += step;
    }

    for (int i = 0; i < X_VECTORS;) {
        x_vector[i++] = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
    }

    for (int i = 0; i < Y_VECTORS;) {
        y_vector[i++] = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
    }

    player_obj = gfx_object_find("player");
    memset(&player, 0, sizeof(player));

    runlevel_register_func(&play1_play2, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&play2_play0, RUNLEVEL_PLAY2, RUNLEVEL_PLAY0,
                           rl_play2_play0);

    runlevel_register_func(&play2_title0, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);

    input_ref = input;
}

// Animate player
static void
animate(struct Player* p, struct MLInput* input)
{
    int frame = p->sprite.frame;

    p->steer_count++;
    if (p->steer_count > count[frame]) {
        if (input->axis_x1 != 0) {
            frame += input->axis_x1;
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
        p->sprite.x -= count[frame];
        if (p->sprite.x < X_MIN) {
            p->sprite.x = X_MIN;
        }
    }
    if (frame > 7) {
        p->sprite.x += count[frame];
        if (p->sprite.x > X_MAX) {
            p->sprite.x = X_MAX;
        }
    }

    collision_update_from_sprite(p->collision, &p->sprite);
}

// Callback function to keep track of how many shots that's in the air.
static void
shot_cb(void)
{
    live_shots--;
}

// Kill pilot.
static void
die(void)
{
    int i, xv, yv, x, y;

    x = player.sprite.x;
    y = player.sprite.y;

    for (i = 0; i < 8; i++) {
        xv = x_vector[x_vector_pos++];
        if (x_vector_pos == X_VECTORS) {
            x_vector_pos = 0;
        }

        do {
            yv = y_vector[y_vector_pos++];
            if (y_vector_pos == Y_VECTORS) {
                y_vector_pos = 0;
            }
        } while ((abs(xv) + abs(yv)) < 3);

        shot_create(x + xv, y + yv, xv, yv, pack_rgb565((struct rgb){.g =  31, .b = 31}), 0, 0, 0);
    }

    if (g_runlevel == RUNLEVEL_PLAY1) {
        g_next_runlevel = RUNLEVEL_PLAY2;
    }

    g_pilots--;
    is_alive = false;

    sfx_player_explode();
}

void
player_kill(void)
{
    if (player.collision) {
        die();
        collision_destroy(player.collision);
        player.collision = 0;
    }
}

static int
collision_cb(struct Collision* a, struct Collision* b)
{
    (void)a;
    (void)b;

    die();
    player.collision = 0;
    return 1;
}

void
player_draw(const struct MLGraphicsBuffer* dst)
{
    if (player.collision) {
        sprite_draw(dst, &player.sprite);
    }
}

// New player
static void
player_new(void)
{
    if (!player.collision) {
        sprite_init(&player.sprite, player_obj, 7, MLDisplayWidth / 2, MLDisplayHeight - 16, 1);
        player.steer_count = 0;
        player.collision = collision_create(0, 0, GID_PLAYER, collision_cb);
        collision_update_from_sprite(player.collision, &player.sprite);
        is_alive = true;
    }
}

// Give up and return to the title screen.
static void
suicide(void)
{
    if (player.collision) {
        collision_destroy(player.collision);
        player.collision = 0;
        g_pilots = 0;
        is_alive = false;
    }
}

void
player_update(struct MLInput* input)
{
    if (g_runlevel >= RUNLEVEL_PLAY0 &&
        g_runlevel <= RUNLEVEL_PLAY2) {

        if (input->press_quit) {
            input->press_quit = false;
            suicide();
        }

        if (g_pilots) {
            if (g_runlevel == RUNLEVEL_PLAY0 && !player.collision) {
                player_new();
            }

            if (player.collision) {
                animate(&player, input);
                move(&player);

                if (input->press_button_a) {
                    input->press_button_a = false;

                    const bool quickshot = false; // For testing
                    if ((!live_shots || quickshot) &&
                        g_runlevel == RUNLEVEL_PLAY1) {
                        if (shot_create(player.sprite.x - 1, player.sprite.y - 16,
                                        0, -8, pack_rgb565(rgb565_color_white()), 1, GID_PLAYER_SHOT,
                                        shot_cb)) {
                            live_shots++;
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
    return is_alive;
}
