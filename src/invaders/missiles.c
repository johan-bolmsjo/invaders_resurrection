/* Missile handling.
 */

#include "missiles.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "armada.h"
#include "gids.h"
#include "runlevel.h"
#include "shields.h"

#define MISSILES_STEP 3  /* Missile speed */
#define MISSILES_MAX  16 /* No more than this many missiles away \
                            on any level */

int g_missiles_alive = 0; /* Missiles alive */

static int missiles_off[MISSILES_MAX]; /* Missiles offset table */
static int missiles_off_i = 0;         /* Missiles offset table index */

static Missile missiles[MISSILES_MAX];

static char g_delays[MISSILES_MAX]; /* Missile drop delays */

static int
collision_cb(Collision* a, Collision* b)
{
    if (b->gid == GID_PLAYER ||
        b->gid == GID_PLAYER_SHOT) {
        missiles[a->id].c = 0;
        g_missiles_alive--;

        return 1;
    }

    if (b->gid == GID_SHIELD) {
        if (shields_hit(a->x0, a->y0, 1, b->id)) {
            missiles[a->id].c = 0;
            g_missiles_alive--;

            return 1;
        }
    }

    return 0;
}

void
missiles_module_init(void)
{
    int i;
    GfxObject* gfx_obj;

    memset(missiles, 0, sizeof(Missile) * MISSILES_MAX);
    gfx_obj = gfx_object_find("missile");

    for (i = 0; i < MISSILES_MAX; i++) {
        missiles_off[i] = (int)((double)ARMADA_X * random() /
                                (RAND_MAX + 1.0)) +
                          1;
        missiles[i].s.vis = 1;
        missiles[i].s.go = gfx_obj;
    }

    for (i = 0; i < MISSILES_MAX; i++)
        g_delays[i] = (int)(15.0 * random() / (RAND_MAX + 1.0)) + 4;
}

void
missiles_show(const DG* dg)
{
    int i;

    for (i = 0; i < MISSILES_MAX; i++) {
        if (missiles[i].c) {
            sprite_show(dg, &missiles[i].s);
        }
    }
}

void
missiles_hide(const DG* dg)
{
    int i;

    for (i = 0; i < MISSILES_MAX; i++)
        sprite_hide(dg, &missiles[i].s);
}

//  TODO(jb): Dependant on armada.c stuff.. Uggly:(
void
missiles_update(void)
{
    static char counters[ARMADA_X] = {0};
    static int x = 0, delay_i = 0;
    int i, y, mi, list[ARMADA_X], alive;

    for (mi = 0; mi < MISSILES_MAX; mi++) {
        if (missiles[mi].c) {
            missiles[mi].s.y += MISSILES_STEP;
            if (missiles[mi].s.y > (MLDisplayWidth + 4)) {
                collision_destroy(missiles[mi].c);
                missiles[mi].c = 0;
                g_missiles_alive--;
            } else {
                collision_update_from_sprite(missiles[mi].c, &missiles[mi].s);
            }
        }
    }

    if (g_runlevel == RUNLEVEL_PLAY1 &&
        g_next_runlevel == RUNLEVEL_PLAY1 &&
        armada.alive) {
        if (armada.missiles_max > MISSILES_MAX)
            armada.missiles_max = MISSILES_MAX;

        mi = 0;
        alive = 0;

        for (i = 0; i < ARMADA_X; i++) {
            if (armada.alive_x[i]) {
                list[alive++] = i;
                if (counters[i])
                    counters[i]--;
            }
        }

        for (i = g_missiles_alive; i < armada.missiles_max; i++) {
            x += missiles_off[missiles_off_i++];
            if (missiles_off_i >= MISSILES_MAX)
                missiles_off_i = 0;

            x = list[x % alive];

            if (!counters[x]) {
                counters[x] = g_delays[delay_i++];
                if (delay_i >= MISSILES_MAX)
                    delay_i = 0;

                while (missiles[mi].c)
                    mi++;

                y = armada.bm;
                while (!armada.b[y][x].c)
                    y--;

                missiles[mi].s.x = armada.b[y][x].s.x;
                missiles[mi].s.y = armada.b[y][x].s.y + 15 + 4;
                g_missiles_alive++;
                missiles[mi].c = collision_create(mi, 0, GID_BOMBER_SHOT,
                                                  collision_cb);
                collision_update_from_sprite(missiles[mi].c, &missiles[mi].s);
            }
        }
    }
}
