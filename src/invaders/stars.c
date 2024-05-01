#include "stars.h"

#include <stdlib.h>
#include <inttypes.h>

#include "libmedia/libmedia.h"
#include "libutil/array.h"
#include "libutil/color.h"

enum {
    NumberOfStars = 384,
    NumberOfStarColors = 32,
    BaseStarSpeed = 32,
};

struct Star {
    struct rgb565 color;
    int           x_off;        // X offset
    int32_t       y_fix;        // Y position (16:16 fix-point)
    int32_t       speed;        // Pixel speed (16:16 fix-point)
};

static struct {
    struct MLRectDim screen_dim;

    int x_random[1022];
    int x_random_pos;
    int y_random[1023];
    int y_random_pos;
    int z_random[1024];
    int z_random_pos;

    struct Star stars[NumberOfStars];

    struct rgb565 stars_cmap[NumberOfStarColors];
} stars_module;
#define M stars_module


static void
create_star(struct Star* star)
{
    int x, y, z;

    do {
        if (M.x_random_pos >= (int)ARRAY_SIZE(M.x_random)) {
            M.x_random_pos = 0;
        }
        if (M.y_random_pos >= (int)ARRAY_SIZE(M.y_random)) {
            M.y_random_pos = 0;
        }
        if (M.z_random_pos >= (int)ARRAY_SIZE(M.z_random)) {
            M.z_random_pos = 0;
        }

        y = M.y_random[M.y_random_pos++];
        z = M.z_random[M.z_random_pos++];

        x = M.x_random[M.x_random_pos++];
        x = ((int32_t)256 * x) / z + (M.screen_dim.w / 2);
    } while (x < 0 || x >= M.screen_dim.w);

    const int32_t a = (int32_t)((double)(256 * 65536 * y) / z + ((double)M.screen_dim.h / 2));
    const int32_t b = (int32_t)((double)(256 * 65536 * (y + BaseStarSpeed)) / z + ((double)M.screen_dim.h / 2));

    star->color = M.stars_cmap[z >> 9];
    star->y_fix = a;
    star->speed = b - a;
    star->x_off = x;
}

void
stars_module_init(struct MLRectDim screen_dim)
{
    M.screen_dim = screen_dim;

    // 320/256 * "max z". -20480 ... 20479
    for (size_t i = 0; i < ARRAY_SIZE(M.x_random); i++) {
        M.x_random[i] = (int)((40960.0 * random() / (RAND_MAX + 1.0)) - 20480);
    }

    // Above camera. -16384 ... -32767
    for (size_t i = 0; i < ARRAY_SIZE(M.y_random); i++) {
        M.y_random[i] = (int)(16384.0 * random() / (RAND_MAX + 1.0)) - 32767;
    }

    // No divide by zero. 1 ... 16383
    for (size_t i = 0; i < ARRAY_SIZE(M.z_random); i++) {
        M.z_random[i] = (int)(16383.0 * random() / (RAND_MAX + 1.0)) + 1;
    }

    // Make the first stars black to avoid the "star belt" effect when they first appear.
    for (size_t i = 0; i < ARRAY_SIZE(M.stars_cmap); i++) {
        M.stars_cmap[i] = pack_rgb565(rgb565_color_black());
    }
    for (size_t i = 0; i < ARRAY_SIZE(M.stars); i++) {
        create_star(&M.stars[i]);
    }

    // Just grey scales for now.
    for (size_t i = 0; i < ARRAY_SIZE(M.stars_cmap); i++) {
        M.stars_cmap[ARRAY_SIZE(M.stars_cmap) - i - 1] = (struct rgb565){i << shift_rgb565_r | (i * 2) << shift_rgb565_g | i};
    }

}

void
stars_draw(const struct MLGraphicsBuffer* dst)
{
    for (size_t i = 0; i < ARRAY_SIZE(M.stars); i++) {
        struct Star* star = &M.stars[i];
        star->y_fix += star->speed;

        const int y = star->y_fix >> 16;
        if (y < 0) {
            continue;
        }
        if (y >= dst->dim.h) {
            create_star(star);
            continue;
        }
        struct rgb565* plot_addr = ml_graphics_buffer_xy(dst, star->x_off, y);
        *plot_addr = star->color;
    }
}
