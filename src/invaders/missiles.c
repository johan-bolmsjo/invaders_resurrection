// Missile handling.

#include "missiles.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "armada.h"
#include "gids.h"
#include "libutil/array.h"
#include "libutil/prng.h"
#include "runlevel.h"
#include "shields.h"

enum {
    MissileSpeed = 3,
    MaxMissileCount = 16,
};

int g_missiles_alive = 0;

static struct {
    struct prng64_state* prng_state;
    struct MLRectDim screen_dim;

    struct Missile missiles[MaxMissileCount];

    // Missile drop delays
    char delays[MaxMissileCount];

    char counters[ArmadaWidth];
    int delay_i;
    int x;
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
missiles_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state)
{
    M.prng_state = prng_state;
    M.screen_dim = screen_dim;

    struct GfxObject* gfx_obj = gfx_object_find("missile");
    for (size_t i = 0; i < ARRAY_SIZE(M.missiles); i++) {
        M.missiles[i].sprite.show = true;
        M.missiles[i].sprite.gfx_obj = gfx_obj;
    }

    for (size_t i = 0; i < MaxMissileCount; i++) {
        M.delays[i] = (int)(prng64_next_double(M.prng_state) * 15 + 4);
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
    for (int mi = 0; mi < MaxMissileCount; mi++) {
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

        int mi = 0;
        int alive_x_count = 0;
        int alive_x_index[ArmadaWidth];

        for (int i = 0; i < ArmadaWidth; i++) {
            if (armada.alive_x[i]) {
                alive_x_index[alive_x_count++] = i;
                if (M.counters[i]) {
                    M.counters[i]--;
                }
            }
        }

        for (int i = g_missiles_alive; i < armada.missiles_max; i++) {
            M.x += (int)(10 * prng64_next_double(M.prng_state)) + 1; // interval [1, 10]
            M.x = alive_x_index[M.x % alive_x_count];

            if (!M.counters[M.x]) {
                M.counters[M.x] = M.delays[M.delay_i++];
                if (M.delay_i >= MaxMissileCount) {
                    M.delay_i = 0;
                }

                while (M.missiles[mi].collision) {
                    mi++;
                }

                int y = armada.bm;
                while (!armada.b[y][M.x].collision) {
                    y--;
                }

                M.missiles[mi].sprite.x = armada.b[y][M.x].sprite.x;
                M.missiles[mi].sprite.y = armada.b[y][M.x].sprite.y + 15 + 4;
                g_missiles_alive++;
                M.missiles[mi].collision = collision_create(mi, 0, GID_BOMBER_SHOT, collision_cb);
                collision_update_from_sprite(M.missiles[mi].collision, &M.missiles[mi].sprite);
            }
        }
    }
}
