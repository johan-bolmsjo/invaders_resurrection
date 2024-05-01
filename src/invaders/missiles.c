// Missile handling.

#include "missiles.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "armada.h"
#include "gids.h"
#include "runlevel.h"
#include "shields.h"

enum {
    MissileSpeed = 3,
    MaxMissileCount = 16,
};

int g_missiles_alive = 0;

static struct {
    struct MLRectDim screen_dim;

    // Missiles offset table and index
    int missiles_off[MaxMissileCount];
    int missiles_off_i;

    struct Missile missiles[MaxMissileCount];

    // Missile drop delays
    char delays[MaxMissileCount];

} missiles_module;
#define M missiles_module


static int
collision_cb(struct Collision* a, struct Collision* b)
{
    if (b->gid == GID_PLAYER || b->gid == GID_PLAYER_SHOT) {
        M.missiles[a->id].collision = 0;
        g_missiles_alive--;
        return 1;
    }

    if (b->gid == GID_SHIELD) {
        if (shields_hit(a->x0, a->y0, 1, b->id)) {
            M.missiles[a->id].collision = 0;
            g_missiles_alive--;
            return 1;
        }
    }

    return 0;
}

void
missiles_module_init(struct MLRectDim screen_dim)
{
    M.screen_dim = screen_dim;

    struct GfxObject* gfx_obj = gfx_object_find("missile");
    for (size_t i = 0; i < MaxMissileCount; i++) {
        M.missiles_off[i] = (int)((double)ARMADA_X * random() / (RAND_MAX + 1.0)) + 1;
        M.missiles[i].sprite.show = true;
        M.missiles[i].sprite.gfx_obj = gfx_obj;
    }

    for (size_t i = 0; i < MaxMissileCount; i++) {
        M.delays[i] = (int)(15.0 * random() / (RAND_MAX + 1.0)) + 4;
    }
}

void
missiles_draw(const struct MLGraphicsBuffer* dst)
{
    int i;

    for (i = 0; i < MaxMissileCount; i++) {
        if (M.missiles[i].collision) {
            sprite_draw(dst, &M.missiles[i].sprite);
        }
    }
}

//  TODO(jb): Dependant on armada.c stuff.. Uggly:(
void
missiles_update(void)
{
    static char counters[ARMADA_X] = {0};
    static int x = 0, delay_i = 0;
    int i, y, mi, list[ARMADA_X], alive;

    for (mi = 0; mi < MaxMissileCount; mi++) {
        if (M.missiles[mi].collision) {
            M.missiles[mi].sprite.y += MissileSpeed;
            if (M.missiles[mi].sprite.y > (M.screen_dim.w + 4)) {
                collision_destroy(M.missiles[mi].collision);
                M.missiles[mi].collision = 0;
                g_missiles_alive--;
            } else {
                collision_update_from_sprite(M.missiles[mi].collision, &M.missiles[mi].sprite);
            }
        }
    }

    if (g_runlevel == RUNLEVEL_PLAY1 &&
        g_next_runlevel == RUNLEVEL_PLAY1 &&
        armada.alive) {
        if (armada.missiles_max > MaxMissileCount) {
            armada.missiles_max = MaxMissileCount;
        }

        mi = 0;
        alive = 0;

        for (i = 0; i < ARMADA_X; i++) {
            if (armada.alive_x[i]) {
                list[alive++] = i;
                if (counters[i]) {
                    counters[i]--;
                }
            }
        }

        for (i = g_missiles_alive; i < armada.missiles_max; i++) {
            x += M.missiles_off[M.missiles_off_i++];
            if (M.missiles_off_i >= MaxMissileCount) {
                M.missiles_off_i = 0;
            }

            x = list[x % alive];

            if (!counters[x]) {
                counters[x] = M.delays[delay_i++];
                if (delay_i >= MaxMissileCount)
                    delay_i = 0;

                while (M.missiles[mi].collision) {
                    mi++;
                }

                y = armada.bm;
                while (!armada.b[y][x].collision) {
                    y--;
                }

                M.missiles[mi].sprite.x = armada.b[y][x].sprite.x;
                M.missiles[mi].sprite.y = armada.b[y][x].sprite.y + 15 + 4;
                g_missiles_alive++;
                M.missiles[mi].collision = collision_create(mi, 0, GID_BOMBER_SHOT, collision_cb);
                collision_update_from_sprite(M.missiles[mi].collision, &M.missiles[mi].sprite);
            }
        }
    }
}
