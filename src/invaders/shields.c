#include "shields.h"

#include <string.h>

#include "collision.h"
#include "gids.h"
#include "libgfx/libgfx.h"
#include "libmedia/libmedia.h"
#include "libutil/xmath.h"
#include "libutil/color.h"
#include "prim.h"
#include "runlevel.h"
#include "sprite.h"

enum {
    shield_count = 4,
    shield_x_dim = 60,
    shield_y_dim = 44,

    // Black border around shield before convolution matrix is applied.
    // The convolution matrix will grow the shield nearer its perimeter.
    shield_border = 4,

    // Number of shield flashing animation frames.
    shield_flash_frames = 2,
};

typedef struct rgb565 shield_type[shield_y_dim][shield_x_dim];

// Shields for flashing animation
static shield_type g_shield_orig1;
static shield_type g_shield_orig2;
static shield_type* g_shields_flash[shield_flash_frames] = {&g_shield_orig1, &g_shield_orig2};

// Pristine shield
static shield_type g_shield_orig3;

// Shields subject to erosion
static shield_type g_shield1;
static shield_type g_shield2;
static shield_type g_shield3;
static shield_type g_shield4;
static shield_type* g_shields[shield_count] = {&g_shield1, &g_shield2, &g_shield3, &g_shield4};

static Collision* g_collisions[shield_count] = {0};

static Clip g_clip1 = {128 - shield_x_dim / 2, MLDisplayHeight - 80 - shield_y_dim / 2,
                       shield_x_dim, shield_y_dim, 0, 0};
static Clip g_clip2 = {256 - shield_x_dim / 2, MLDisplayHeight - 80 - shield_y_dim / 2,
                       shield_x_dim, shield_y_dim, 0, 0};
static Clip g_clip3 = {384 - shield_x_dim / 2, MLDisplayHeight - 80 - shield_y_dim / 2,
                       shield_x_dim, shield_y_dim, 0, 0};
static Clip g_clip4 = {512 - shield_x_dim / 2, MLDisplayHeight - 80 - shield_y_dim / 2,
                       shield_x_dim, shield_y_dim, 0, 0};

static Clip* g_clips[shield_count] = {&g_clip1, &g_clip2, &g_clip3, &g_clip4};

static bool g_draw[shield_count]  = {false};

// Used for the flashing animation.
static uint8_t g_flash_count[shield_count] = {
    shield_flash_frames, shield_flash_frames, shield_flash_frames, shield_flash_frames
};

// Collision callback routine.
static int
collision_cb(Collision* a, Collision* b)
{
    if (b->gid == GID_BOMBER) {
        g_draw[a->id] = false;
        g_flash_count[a->id] = 0;
    }

    return 0;
}

// Create shield data and other initialisations.
void
shield_module_init(void)
{
    uint8_t shield_tmpl[shield_y_dim][shield_x_dim] = {{0}};

    // Create a shield template with value 160 for '*':
    //
    //     **********
    //    ************
    //    ************
    //    ************
    //
    int x0 = 8 + shield_border;
    int x1 = shield_x_dim - x0;
    for (int y = shield_border; y < shield_y_dim - shield_border; y++) {
        for (int x = x0; x < x1; x++) {
            shield_tmpl[y][x] = 160;
        }

        if (x0 > shield_border) {
            x0--;
            x1++;
        }
    }

    shield_type* s1 = &g_shield_orig1;
    shield_type* s2 = &g_shield_orig2;
    shield_type* s3 = &g_shield_orig3;

    for (int y = 1; y < shield_y_dim - 1; y++) {
        for (int x = 1; x < shield_x_dim - 1; x++) {
            // A 3x3 convolution matrix that is applied to the shield template to
            // produce anti-aliased edges. The weights sum to 256 so the max value
            // for v is (160 * 256).
            //
            //     4  16  4
            //    16 176 16
            //     4  16  4
            //
            const int v =
                shield_tmpl[y - 1][x - 1] *   4 +
                shield_tmpl[y - 1][x    ] *  16 +
                shield_tmpl[y - 1][x + 1] *   4 +
                shield_tmpl[y    ][x - 1] *  16 +
                shield_tmpl[y    ][x    ] * 176 +
                shield_tmpl[y    ][x + 1] *  16 +
                shield_tmpl[y + 1][x - 1] *   4 +
                shield_tmpl[y + 1][x    ] *  16 +
                shield_tmpl[y + 1][x + 1] *   4;

            const int g = v >> 12; // max value: 10
            const int b = v >> 11; // max value: 20

            // Bulid shields used for the flashing animation
            if (g || b) {
                (*s1)[y][x] = pack_rgb565(rgb_white());
                (*s2)[y][x] = pack_rgb565((struct rgb){15, (g + max_g) / 2, (b + max_b) / 2});
            } else {
                (*s1)[y][x] = pack_rgb565(rgb_black());
                (*s2)[y][x] = pack_rgb565(rgb_black());
            }

            // Build regular shield
            (*s3)[y][x] = pack_rgb565((struct rgb){.g = g, .b = b});
        }
    }
}

void
shields_new(void)
{
    for (int i = 0; i < shield_count; i++) {
        g_draw[i] = true;
        g_flash_count[i] = 0;

        if (!g_collisions[i]) {
            g_collisions[i] = collision_create(i, 0, GID_SHIELD, collision_cb);
            g_collisions[i]->x0 = (i + 1) * 128 - shield_x_dim / 2;
            g_collisions[i]->x1 = (i + 1) * 128 + shield_x_dim / 2 - 1;
            g_collisions[i]->y0 = MLDisplayHeight - 80 - shield_y_dim / 2;
            g_collisions[i]->y1 = MLDisplayHeight - 80 + shield_y_dim / 2 - 1;
        }

        memcpy(g_shields[i], g_shield_orig3, sizeof(g_shield_orig3));
    }
}

void
shields_del(void)
{
    for (int i = 0; i < shield_count; i++) {
        g_draw[i] = false;
        g_flash_count[i] = 0; /* Used to clear two times. */
    }
}

static inline bool
shield_xy_in_bounds(int x, int y) {
    // Silly needless optimization; negative values become large positive values that
    // are still out of bounds.
    return (unsigned)x < shield_x_dim && (unsigned)y < shield_y_dim;
}

static inline struct rgb565
shield_xy_get(shield_type shield, int x, int y) {
    if (shield_xy_in_bounds(x, y)) {
        return shield[y][x];
    }
    return pack_rgb565(rgb_black());
}

static inline void
shield_xy_put(shield_type shield, int x, int y, struct rgb565 c) {
    if (shield_xy_in_bounds(x, y)) {
        shield[y][x] = c;
    }
}

// Make hole in shield from missile or bomb.
static void
make_hole(shield_type shield, int x, int y)
{
    enum {
        x_dim  = 9,
        y_dim = 9,
    };

    // Alpha values:
    //    0 = opaque
    //   16 = transparent
    static uint8_t hole_alpha[y_dim][x_dim] =
        {{16, 8, 8, 8, 8, 8, 8, 8, 16},
         { 8, 4, 0, 0, 0, 0, 0, 4,  8},
         { 8, 0, 0, 0, 0, 0, 0, 0,  8},
         { 8, 0, 0, 0, 0, 0, 0, 0,  8},
         { 8, 0, 0, 0, 0, 0, 0, 0,  8},
         { 8, 0, 0, 0, 0, 0, 0, 0,  8},
         { 8, 0, 0, 0, 0, 0, 0, 0,  8},
         { 8, 4, 0, 0, 0, 0, 0, 4,  8},
         {16, 8, 8, 8, 8, 8, 8, 8, 16}};

    // Adjust xy to center hole
    x -= 4;
    y -= 4;

    for (int hy = 0; hy < y_dim; hy++) {
        for (int hx = 0; hx < x_dim; hx++) {
            const int a = hole_alpha[hy][hx];
            const struct rgb c = unpack_rgb565(shield_xy_get(shield, x + hx, y + hy));
            shield_xy_put(shield, x + hx, y + hy, pack_rgb565((struct rgb){
                        .r = (c.r * a) >> 4,
                        .g = (c.g * a) >> 4,
                        .b = (c.b * a) >> 4,
                    }));
        }
    }
}

bool
shields_hit(int x, int y, int y_vec, int shield_id)
{
    x = clamp_int(x - g_collisions[shield_id]->x0, 0, shield_x_dim - 1);
    y = clamp_int(y - g_collisions[shield_id]->y0, 0, shield_y_dim -1);

    shield_type* shield = g_shields[shield_id];

    // Drill through until a non-black pixel is found.
    while (shield_xy_in_bounds(x, y)) {
        if (shield_xy_get(*shield, x, y).v) {
            make_hole(*shield, x, y);
            return true;
        }
        y += y_vec;
    }
    return false;
}

void
shields_show(const DG* dg)
{
    for (int i = 0; i < shield_count; i++) {
        if (g_draw[i]) {
            if (g_flash_count[i] < shield_flash_frames) {
                blit_clipped_gfx_box(dg, g_clips[i], &(*g_shields_flash[g_flash_count[i]])[0][0].v);
                g_flash_count[i]++;
            } else {
                blit_clipped_gfx_box(dg, g_clips[i], &(*g_shields[i])[0][0].v);
            }
        }
    }
}

void
shields_hide(const DG* dg)
{
    for (int i = 0; i < shield_count; i++) {
        if (!g_draw[i] && g_flash_count[i] < shield_flash_frames) {
            if (g_collisions[i]) {
                collision_destroy(g_collisions[i]);
                g_collisions[i] = 0;
            }

            blit_clipped_colour_box(dg, g_clips[i], 0);
            g_flash_count[i]++;
        }
    }
}
