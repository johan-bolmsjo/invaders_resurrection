#include "armada.h"

#include <stdlib.h>
#include <string.h>

#include "gids.h"
#include "libgfx/libgfx.h"
#include "libutil/array.h"
#include "libutil/prng.h"
#include "player.h"
#include "runlevel.h"
#include "sfx.h"
#include "shields.h"
#include "shot.h"
#include "status.h"

enum {
    BomberWidth     = 36,
    BomberHeight    = 30,
    MoveDistanceX  = 4,
    MoveDistanceY  = 30,
    StartPositionX = 122,
};


// armada is used by missiles.c as well
struct Armada armada;

static struct {
    struct prng64_state* prng_state;
    struct MLRectDim screen_dim;
    struct GfxObject* gfx_obj[3];
} armada_module;
#define M armada_module

#define MaxPositionX (M.screen_dim.w - 18)
#define MinPositionX (18)
#define MaxPositionY (M.screen_dim.h - 15)
#define MinPositionY (MaxPositionY - (11 * BomberHeight))

static int
random_speed(void) {
    return 12 * (prng64_next_double(M.prng_state) - 0.5);
}

// Collision handler.
// Increases score, speed and plays explosion sound.
static int
collision_cb(struct Collision* a, struct Collision* b)
{
    if (b->gid != GID_PLAYER_SHOT) {
        return 0;
    }

    struct Bomber* bomber = (struct Bomber*)a->id_p;

    for (int i = 0; i < 5; i++) {
        int xv = random_speed();
        int yv;
        do {
            yv = random_speed();
        } while ((abs(xv) + abs(yv)) < 3);

        shot_create(bomber->sprite.x + xv, bomber->sprite.y + yv, xv, yv,
                    pack_rgb565((struct rgb){.g = 31}), 0, 0, 0);
    }

    bomber->collision = 0;
    const int x = bomber->x;
    const int y = bomber->y;

    if (y == 0) {
        g_score += 30;
    } else if (y <= 3) {
        g_score += 20;
    } else {
        g_score += 10;
    }

    armada.alive--;

    if (armada.alive) {
        armada.alive_x[x]--;
        armada.alive_y[y]--;

        if (!armada.alive_x[x]) {
            for (; armada.lm <= armada.rm; armada.lm++) {
                if (armada.alive_x[armada.lm]) {
                    break;
                }
            }

            for (; armada.rm >= armada.lm; armada.rm--) {
                if (armada.alive_x[armada.rm]) {
                    break;
                }
            }
        }

        if (!armada.alive_y[y]) {
            for (; armada.tm <= armada.bm; armada.tm++) {
                if (armada.alive_y[armada.tm]) {
                    break;
                }
            }

            for (; armada.bm >= armada.tm; armada.bm--) {
                if (armada.alive_y[armada.bm]) {
                    break;
                }
            }

            armada.rows--;
        }
    }

    sfx_bomber_explode();

    return 1;
}

// Animate armada and update collision objects.
static void
animate(void)
{
    struct Bomber* b = armada.b[0];
    for (int i = 0; i < ArmadaArea; i++) {
        if (b[i].collision) {
            bomber_anim(&b[i]);
            collision_update_from_sprite(b[i].collision, &b[i].sprite);
        }
    }
}

// Move armada to the start position.
static void
move_armada_to_start(void)
{
    int y = MinPositionY + armada.y_off * BomberHeight;

    for (int i = 0; i < ArmadaHeight; i++) {
        int x = StartPositionX;
        for (int j = 0; j < ArmadaWidth; j++) {
            armada.b[i][j].sprite.x = x;
            armada.b[i][j].sprite.y = y;
            x += BomberWidth;
        }
        y += BomberHeight;
    }

    armada.row = armada.bm;
    armada.row_c = 0;
    armada.row_cw = armada.alive / armada.rows;
    armada.frac = armada.alive % armada.rows;
    armada.kill = 0;
}

// Move armada in the usual space invaders way.
static void
move_armada(void)
{
    static int sfx_counter = 0;

    if (!armada.alive) {
        return;
    }

    sfx_counter++;

    if (armada.frac) {
        armada.frac--;
        return;
    }

    armada.row_c++;
    if (armada.row_c >= armada.row_cw) {
        armada.row_c = 0;

        if (armada.row < 0) {
            armada.row = armada.bm;
        }

        while (!armada.alive_y[armada.row]) {
            armada.row--;
            if (armada.row < 0) {
                armada.row = armada.bm;
            }
        }

        if (armada.row == armada.tm) {
            armada.row_cw = armada.alive / armada.rows;
            armada.frac = armada.alive % armada.rows;
        }

        int x = 0;
        if (armada.dir_r) {
            if (armada.b[armada.row][armada.rm].sprite.x + MoveDistanceX <= MaxPositionX) {
                x = MoveDistanceX;
            } else {
                armada.dir_d = 1;
            }
        } else {
            if (armada.b[armada.row][armada.lm].sprite.x - MoveDistanceX >= MinPositionX) {
                x = -MoveDistanceX;
            } else {
                armada.dir_d = 1;
            }
        }

        int y = 0;
        if (armada.dir_d) {
            if (armada.b[armada.row][armada.lm].sprite.y + MoveDistanceY == MaxPositionY) {
                armada.kill = 1;
            }

            if (armada.row == armada.tm) {
                armada.dir_d = 0;
                armada.dir_r ^= 1;

                if (armada.kill) {
                    player_kill();
                    armada.kill = 0;
                }
            }

            y = MoveDistanceY;
        }

        if (x || y) {
            if (armada.row == armada.bm && sfx_counter >= 2) {
                sfx_bomber_move();
                sfx_counter = 0;
            }

            for (size_t i = 0; i < ArmadaWidth; i++) {
                armada.b[armada.row][i].sprite.x += x;
                armada.b[armada.row][i].sprite.y += y;
            }

            armada.row--;
        }
    }
}

void
armada_module_init(struct MLRectDim screen_dim, struct prng64_state* prng_state)
{
    M.prng_state = prng_state;
    M.screen_dim = screen_dim;

    memset(&armada, 0, sizeof(armada));

    M.gfx_obj[0] = gfx_object_find("bomber_3");
    M.gfx_obj[1] = gfx_object_find("bomber_2");
    M.gfx_obj[2] = gfx_object_find("bomber_1");

    int type = 0;

    for (int y = 0; y < ArmadaHeight; y++) {
        if (y == 1) {
            type = 1;
        }
        if (y == 3) {
            type = 2;
        }

        for (int x = 0; x < ArmadaWidth; x++) {
            armada.b[y][x].x = x;
            armada.b[y][x].y = y;
            armada.b[y][x].sprite.show = false;
            armada.b[y][x].sprite.gfx_obj = M.gfx_obj[type];
        }
    }
}

// Create new armada.
//
// TODO(jb): Doesn't update the collision positions so be sure to run animate()
//           before collision_detection() is run.
//
static void
armada_new(void)
{
    if (armada.alive) {
        return;
    }

    armada.y_off++;
    if (armada.y_off > 2) {
        armada.y_off = 0;
        armada.missiles_max++;
    }

    for (int i = 0; i < ArmadaWidth; i++) {
        armada.alive_x[i] = ArmadaHeight;
    }

    for (int i = 0; i < ArmadaHeight; i++) {
        armada.alive_y[i] = ArmadaWidth;
    }

    struct Bomber* b = armada.b[0];
    for (int i = 0; i < ArmadaArea; i++) {
        b[i].count = 0;
        b[i].sprite.frame = 0;
        b[i].collision = collision_create(0, (void*)&b[i], GID_BOMBER, collision_cb);
    }

    armada.lm = 0;
    armada.rm = ArmadaWidth - 1;
    armada.tm = 0;
    armada.bm = ArmadaHeight - 1;

    armada.alive = ArmadaArea;
    armada.rows = ArmadaHeight;
    armada.vis_c = 0;
    armada.dir_r = 1;
    armada.dir_d = 0;

    move_armada_to_start();
}

void
armada_reset(void)
{
    armada.y_off = -1; // armada_new () is called after this function
    armada.missiles_max = 3;
}

void
armada_draw(const struct MLGraphicsBuffer* dst)
{
    struct Bomber* b = armada.b[0];
    for (int i = 0; i < ArmadaArea; i++) {
        if (b[i].collision && b[i].sprite.show) {
            sprite_draw(dst, &b[i].sprite);
        }
    }
}

void
armada_update(void)
{
    if (g_runlevel == RUNLEVEL_PLAY0) {
        if (!armada.alive) {
            armada_new();
            shields_new();
        }

        if (armada.vis_c < ArmadaArea) {
            armada.b[0][armada.vis_c++].sprite.show = true;
        } else {
            g_next_runlevel = RUNLEVEL_PLAY1;
        }
    }

    if (g_runlevel == RUNLEVEL_PLAY1) {
        if (armada.alive) {
            move_armada();
        } else {
            g_next_runlevel = RUNLEVEL_PLAY2;
        }
    }

    if (g_runlevel == RUNLEVEL_PLAY2) {
        if (armada.vis_c) {
            struct Bomber*  b = &armada.b[0][--armada.vis_c];
            b->sprite.show = false;
        } else {
            if (g_pilots) {
                g_next_runlevel = RUNLEVEL_PLAY0;
                move_armada_to_start();
            } else {
                struct Bomber* b = armada.b[0];

                for (int i = 0; i < ArmadaArea; i++) {
                    if (b[i].collision) {
                        collision_destroy(b[i].collision);
                        b[i].collision = 0;
                    }
                }

                armada.alive = 0;
                shields_del();
            }
        }
    }

    animate();
}
