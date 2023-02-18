#include "player.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gids.h"
#include "missiles.h"
#include "runlevel.h"
#include "sfx.h"
#include "shot.h"
#include "status.h"

#define X_VECTORS 15
#define Y_VECTORS 16

#define X_MIN 60
#define X_MAX (DG_XRES - 60)

int g_player_alive = 0;

static int x_vector[X_VECTORS];
static int y_vector[Y_VECTORS];
static int x_vector_pos = 0;
static int y_vector_pos = 0;

static int count[15];
static int shots = 0; /* Shots in the air */
static Player player; /* Player information */
static GfxObject* player_obj;

static Joy* joy_p; /* Set in player_tables(), used by runlevel func */

extern int g_cheat; /* Cheat flag from main.c */

/* Make sure that no shots are alive before
 * changing from runlevel play1 to play2.
 */

static int
rl_play1_play2(RunLevelFunc* r)
{
    (void)r;

    if (!shots)
        return 1;

    return 0;
}

/* Make sure that no missiles are alive before
 * changing from runlevel play2 to play0.
 */

static int
rl_play2_play0(RunLevelFunc* r)
{
    (void)r;

    if (!g_missiles_alive)
        return 1;

    return 0;
}

/* Make sure that no objects or shots are alive before
 * changing from runlevel play2 to title0.
 *
 * Also clear joystick button flag.
 */

static int
rl_play2_title0(RunLevelFunc* r)
{
    (void)r;

    static int pend = 0;

    if (!(g_collision_obj + g_shot_obj))
        pend++;

    if (pend == 2) {
        pend = 0;
        return 1;
    }

    joy_p->button = 0;

    return 0;
}

/* Create tables etc.
 */

void
player_tables(Joy* joy)
{
    int i, j;
    double step = 1.5708 / 7, pos = -1.5708;
    static RunLevelFunc play1_play2;
    static RunLevelFunc play2_play0;
    static RunLevelFunc play2_title0;

    for (i = 0; i < 15; i++) {
        count[i] = floor(2 * sin(fabs(pos)) + 0.5);
        pos += step;
    }

    for (i = 0; i < X_VECTORS;) {
        j = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
        x_vector[i++] = j;
    }

    for (i = 0; i < Y_VECTORS;) {
        j = (int)(11.0 * random() / (RAND_MAX + 1.0)) - 5;
        y_vector[i++] = j;
    }

    player_obj = gfx_object_find("player");
    memset(&player, 0, sizeof(Player));

    runlevel_register_func(&play1_play2, RUNLEVEL_PLAY1, RUNLEVEL_PLAY2,
                           rl_play1_play2);

    runlevel_register_func(&play2_play0, RUNLEVEL_PLAY2, RUNLEVEL_PLAY0,
                           rl_play2_play0);

    runlevel_register_func(&play2_title0, RUNLEVEL_PLAY2, RUNLEVEL_TITLE0,
                           rl_play2_title0);

    joy_p = joy;
}

/* Animate player
 */

static void
animate(Player* p, Joy* j)
{
    int frame = p->s.frame;

    p->count++;
    if (p->count > count[frame]) {
        if (j->x_axis) {
            frame += j->x_axis;
            if (frame < 0)
                frame = 0;
            if (frame > 14)
                frame = 14;
        } else {
            if (frame < 7)
                frame++;
            if (frame > 7)
                frame--;
        }
        p->count = 0;
    }

    p->s.frame = frame;
}

/* Move player
 */

static void
move(Player* p)
{
    int frame = p->s.frame;

    if (frame < 7) {
        p->s.x -= count[frame];
        if (p->s.x < X_MIN)
            p->s.x = X_MIN;
    }
    if (frame > 7) {
        p->s.x += count[frame];
        if (p->s.x > X_MAX)
            p->s.x = X_MAX;
    }

    collision_update_from_sprite(p->c, &p->s);
}

/* Callback function to keep track of how many shots that's
 * in the air.
 */

static void
shot_cb()
{
    shots--;
}

/* Kill pilot.
 */

static void
die()
{
    int i, xv, yv, x, y;

    x = player.s.x;
    y = player.s.y;

    for (i = 0; i < 8; i++) {
        xv = x_vector[x_vector_pos++];
        if (x_vector_pos == X_VECTORS)
            x_vector_pos = 0;

        do {
            yv = y_vector[y_vector_pos++];
            if (y_vector_pos == Y_VECTORS)
                y_vector_pos = 0;
        } while ((abs(xv) + abs(yv)) < 3);

        shot_create(x + xv, y + yv, xv, yv, 31 << 5 | 31, 0, 0, 0);
    }

    if (g_runlevel == RUNLEVEL_PLAY1)
        g_next_runlevel = RUNLEVEL_PLAY2;

    g_pilots--;
    g_player_alive = 0;

    sfx_player_explode();
}

/* Kill pilot and destroy its collision object.
 */

void
player_kill()
{
    if (player.c) {
        die();
        collision_destroy(player.c);
        player.c = 0;
    }
}

/* Collision callback.
 */

static int
collision_cb(Collision* a, Collision* b)
{
    (void)a;
    (void)b;

    die();
    player.c = 0;
    return 1;
}

/* Hide player
 */

void
player_hide(DG* dg)
{
    sprite_hide(dg, &player.s);
}

/* Show player
 */

void
player_show(DG* dg)
{
    if (player.c)
        sprite_show(dg, &player.s);
}

/* New player
 */

static void
player_new()
{
    if (!player.c) {
        sprite_init(&player.s, player_obj, 7, DG_XRES / 2, DG_YRES - 16, 1);
        player.count = 0;
        player.c = collision_create(0, 0, GID_PLAYER, collision_cb);
        collision_update_from_sprite(player.c, &player.s);
        g_player_alive = 1;
    }
}

/* Give up and return to the title screen.
 */

static void
suicide()
{
    if (player.c) {
        collision_destroy(player.c);
        player.c = 0;
        g_pilots = 0;
        g_player_alive = 0;
    }
}

/* Player handler function.
 */

void
player_update(Joy* j, int* key_q)
{
    if (g_runlevel >= RUNLEVEL_PLAY0 &&
        g_runlevel <= RUNLEVEL_PLAY2) {

        if (*key_q) {
            *key_q = 0;
            suicide();
        }

        if (g_pilots) {
            if (g_runlevel == RUNLEVEL_PLAY0 && !player.c)
                player_new();

            if (player.c) {
                animate(&player, j);
                move(&player);

                if (j->button) {
                    j->button = 0;

                    if ((!shots || g_cheat) &&
                        g_runlevel == RUNLEVEL_PLAY1) {
                        if (shot_create(player.s.x - 1, player.s.y - 16,
                                        0, -8, 0xFFFF, 1, GID_PLAYER_SHOT,
                                        shot_cb))
                            shots++;

                        sfx_player_shot();
                    }
                }
            }
        } else {
            if (g_runlevel == RUNLEVEL_PLAY0 ||
                g_runlevel == RUNLEVEL_PLAY1)
                g_next_runlevel = RUNLEVEL_PLAY2;

            if (g_runlevel == RUNLEVEL_PLAY2)
                g_next_runlevel = RUNLEVEL_TITLE0;
        }
    }
}
