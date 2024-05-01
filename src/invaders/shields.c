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
    NumberOfShields = 4,
    ShieldWidth = 60,
    ShieldHeight = 44,

    // Black border around shield before convolution matrix is applied.
    // The convolution matrix will grow the shield nearer its perimeter.
    ShieldBorderSize = 4,

    // Number of shield flashing animation frames.
    NumberOfShieldFlashFrames = 2,
};

typedef struct rgb565 shield_type[ShieldHeight][ShieldWidth];

static struct {
    struct MLRectDim screen_dim;

    // Shields for flashing animation
    shield_type shield_frame1;
    shield_type shield_frame2;
    shield_type* g_shields_flash[NumberOfShieldFlashFrames];

    // Pristine shield
    shield_type shield_frame3;

    // Shields subject to erosion
    shield_type  shield1;
    shield_type  shield2;
    shield_type  shield3;
    shield_type  shield4;
    shield_type* shields[NumberOfShields];

    struct Collision* collisions[NumberOfShields];

    struct Clip  clip1;
    struct Clip  clip2;
    struct Clip  clip3;
    struct Clip  clip4;
    struct Clip* clips[NumberOfShields];

    bool g_draw[NumberOfShields];
} shields_module;
#define M shields_module


// Used for the flashing animation.
static uint8_t g_flash_count[NumberOfShields] = {
    NumberOfShieldFlashFrames, NumberOfShieldFlashFrames, NumberOfShieldFlashFrames, NumberOfShieldFlashFrames
};

// Collision callback routine.
static int
collision_cb(struct Collision* a, struct Collision* b)
{
    if (b->gid == GID_BOMBER) {
        M.g_draw[a->id] = false;
        g_flash_count[a->id] = 0;
    }

    return 0;
}

// Create shield data and other initialisations.
void
shield_module_init(struct MLRectDim screen_dim)
{
    M.screen_dim = screen_dim;

    M.g_shields_flash[0] = &M.shield_frame1;
    M.g_shields_flash[1] = &M.shield_frame2;

    M.shields[0] = &M.shield1;
    M.shields[1] = &M.shield2;
    M.shields[2] = &M.shield3;
    M.shields[3] = &M.shield4;

    M.clip1 = (struct Clip){128 - ShieldWidth / 2, M.screen_dim.h - 80 - ShieldHeight / 2, ShieldWidth, ShieldHeight, 0, 0};
    M.clip2 = (struct Clip){256 - ShieldWidth / 2, M.screen_dim.h - 80 - ShieldHeight / 2, ShieldWidth, ShieldHeight, 0, 0};
    M.clip3 = (struct Clip){384 - ShieldWidth / 2, M.screen_dim.h - 80 - ShieldHeight / 2, ShieldWidth, ShieldHeight, 0, 0};
    M.clip4 = (struct Clip){512 - ShieldWidth / 2, M.screen_dim.h - 80 - ShieldHeight / 2, ShieldWidth, ShieldHeight, 0, 0};
    M.clips[0] = &M.clip1;
    M.clips[1] = &M.clip2;
    M.clips[2] = &M.clip3;
    M.clips[3] = &M.clip4;

    uint8_t shield_tmpl[ShieldHeight][ShieldWidth] = {{0}};

    // Create a shield template with value 160 for '*':
    //
    //     **********
    //    ************
    //    ************
    //    ************
    //
    int x0 = 8 + ShieldBorderSize;
    int x1 = ShieldWidth - x0;
    for (int y = ShieldBorderSize; y < ShieldHeight - ShieldBorderSize; y++) {
        for (int x = x0; x < x1; x++) {
            shield_tmpl[y][x] = 160;
        }

        if (x0 > ShieldBorderSize) {
            x0--;
            x1++;
        }
    }

    shield_type* s1 = &M.shield_frame1;
    shield_type* s2 = &M.shield_frame2;
    shield_type* s3 = &M.shield_frame3;

    for (int y = 1; y < ShieldHeight - 1; y++) {
        for (int x = 1; x < ShieldWidth - 1; x++) {
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
                (*s1)[y][x] = pack_rgb565(rgb565_color_white());
                (*s2)[y][x] = pack_rgb565((struct rgb){15, (g + max_g6) / 2, (b + max_b5) / 2});
            } else {
                (*s1)[y][x] = pack_rgb565(rgb565_color_black());
                (*s2)[y][x] = pack_rgb565(rgb565_color_black());
            }

            // Build regular shield
            (*s3)[y][x] = pack_rgb565((struct rgb){.g = g, .b = b});
        }
    }
}

void
shields_new(void)
{
    for (int i = 0; i < NumberOfShields; i++) {
        M.g_draw[i] = true;
        g_flash_count[i] = 0;

        if (!M.collisions[i]) {
            M.collisions[i] = collision_create(i, 0, GID_SHIELD, collision_cb);
            M.collisions[i]->x0 = (i + 1) * 128 - ShieldWidth / 2;
            M.collisions[i]->x1 = (i + 1) * 128 + ShieldWidth / 2 - 1;
            M.collisions[i]->y0 = M.screen_dim.h - 80 - ShieldHeight / 2;
            M.collisions[i]->y1 = M.screen_dim.h - 80 + ShieldHeight / 2 - 1;
        }

        memcpy(M.shields[i], M.shield_frame3, sizeof(M.shield_frame3));
    }
}

void
shields_del(void)
{
    for (int i = 0; i < NumberOfShields; i++) {
        M.g_draw[i] = false;
        g_flash_count[i] = 0; /* Used to clear two times. */
    }
}

static inline bool
shield_xy_in_bounds(int x, int y) {
    // Silly needless optimization; negative values become large positive values that
    // are still out of bounds.
    return (unsigned)x < ShieldWidth && (unsigned)y < ShieldHeight;
}

static inline struct rgb565
shield_xy_get(shield_type shield, int x, int y) {
    if (shield_xy_in_bounds(x, y)) {
        return shield[y][x];
    }
    return pack_rgb565(rgb565_color_black());
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
    x = clamp_int(x - M.collisions[shield_id]->x0, 0, ShieldWidth - 1);
    y = clamp_int(y - M.collisions[shield_id]->y0, 0, ShieldHeight -1);

    shield_type* shield = M.shields[shield_id];

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
shields_draw(const struct MLGraphicsBuffer* dst)
{
    // TODO(jb): Split this into draw and update functions
    for (int i = 0; i < NumberOfShields; i++) {
        if (M.g_draw[i]) {
            if (g_flash_count[i] < NumberOfShieldFlashFrames) {
                blit_clipped_gfx_box(dst, M.clips[i], &(*M.g_shields_flash[g_flash_count[i]])[0][0]);
                g_flash_count[i]++;
            } else {
                blit_clipped_gfx_box(dst, M.clips[i], &(*M.shields[i])[0][0]);
            }
        } else if (g_flash_count[i] < NumberOfShieldFlashFrames) {
            if (M.collisions[i]) {
                collision_destroy(M.collisions[i]);
                M.collisions[i] = 0;
            }
            g_flash_count[i]++;
        }
    }
}
