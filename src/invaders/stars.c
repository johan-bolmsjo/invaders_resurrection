#include "stars.h"

#include <stdlib.h>
#include <inttypes.h>

#include "libmedia/libmedia.h"
#include "libutil/array.h"
#include "libutil/color.h"

struct Star {
    struct rgb565 color;
    int           x_off;        // X offset
    int32_t       y_fix;        // Y position (16:16 fix-point)
    int32_t       speed;        // Pixel speed (16:16 fix-point)
};

enum {
    NumberOfStars = 384,
    NumberOfStarColors = 32,
    BaseStarSpeed = 32,
};

static int x_random[1022];
static int x_random_pos;
static int y_random[1023];
static int y_random_pos;
static int z_random[1024];
static int z_random_pos;

static struct Star stars[NumberOfStars];

static struct rgb565 stars_cmap[NumberOfStarColors];

static void
create_star(struct Star* star)
{
    int x, y, z;
    int32_t a, b;

    do {
        if (x_random_pos >= (int)ARRAY_SIZE(x_random)) {
            x_random_pos = 0;
        }
        if (y_random_pos >= (int)ARRAY_SIZE(y_random)) {
            y_random_pos = 0;
        }
        if (z_random_pos >= (int)ARRAY_SIZE(z_random)) {
            z_random_pos = 0;
        }

        y = y_random[y_random_pos++];
        z = z_random[z_random_pos++];

        x = x_random[x_random_pos++];
        x = ((int32_t)256 * x) / z + (MLDisplayWidth / 2);
    } while ((unsigned int)x >= MLDisplayWidth);

    a = (int32_t)((double)(256 * 65536 * y) / z + ((double)MLDisplayHeight / 2));
    b = (int32_t)((double)(256 * 65536 * (y + BaseStarSpeed)) / z + ((double)MLDisplayHeight / 2));

    star->color = stars_cmap[z >> 9];
    star->y_fix = a;
    star->speed = b - a;
    star->x_off = x;
}

void
stars_module_init(void)
{
    // 320/256 * "max z". -20480 ... 20479
    for (size_t i = 0; i < ARRAY_SIZE(x_random); i++) {
        x_random[i] = (int)((40960.0 * random() / (RAND_MAX + 1.0)) - 20480);
    }

    // Above camera. -16384 ... -32767
    for (size_t i = 0; i < ARRAY_SIZE(y_random); i++) {
        y_random[i] = (int)(16384.0 * random() / (RAND_MAX + 1.0)) - 32767;
    }

    // No divide by zero. 1 ... 16383
    for (size_t i = 0; i < ARRAY_SIZE(z_random); i++) {
        z_random[i] = (int)(16383.0 * random() / (RAND_MAX + 1.0)) + 1;
    }

    // Make the first stars black to avoid the "star belt" effect when they first appear.
    for (size_t i = 0; i < ARRAY_SIZE(stars_cmap); i++) {
        stars_cmap[i] = pack_rgb565(rgb565_color_black());
    }
    for (size_t i = 0; i < ARRAY_SIZE(stars); i++) {
        create_star(&stars[i]);
    }

    // Just grey scales for now.
    for (size_t i = 0; i < ARRAY_SIZE(stars_cmap); i++) {
        stars_cmap[ARRAY_SIZE(stars_cmap) - i - 1] = (struct rgb565){i << shift_rgb565_r | (i * 2) << shift_rgb565_g | i};
    }

}

void
stars_draw(const struct MLGraphicsBuffer* buf)
{
    for (size_t i = 0; i < ARRAY_SIZE(stars); i++) {
        struct Star* star = &stars[i];
        star->y_fix += star->speed;

        const int y = star->y_fix >> 16;
        if (y < 0) {
            continue;
        }
        if (y >= buf->height) {
            create_star(star);
            continue;
        }
        struct rgb565* plot_addr = ml_graphics_buffer_xy(buf, star->x_off, y);
        *plot_addr = star->color;
    }
}
