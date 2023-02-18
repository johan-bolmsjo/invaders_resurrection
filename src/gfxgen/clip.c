/* Clip windows.
 */

#include <inttypes.h>
#include "image.h"
#include "clip.h"

static uint8_t
pixel_get_alpha(Image* image, int x, int y)
{
    if (image->type != RGBA)
        return 0;

    return image->image[image->depth * x + image->width * image->depth * y + 3];
}

static int
hline_same_alpha(Image* image, int line, uint8_t alpha)
{
    int i;

    for (i = 0; i < image->width; i++) {
        if (pixel_get_alpha(image, i, line) != alpha)
            return 0;
    }

    return 1;
}

static int
vline_same_alpha(Image* image, int line, uint8_t alpha)
{
    int i;

    for (i = 0; i < image->height; i++) {
        if (pixel_get_alpha(image, line, i) != alpha)
            return 0;
    }

    return 1;
}

void
clip_auto_alpha(Image* image, Clip* clip)
{
    int x0, y0, x1, y1, no_clip = 0;

    for (y0 = 0; y0 < image->height; y0++)
        if (!hline_same_alpha(image, y0, 16))
            break;

    for (y1 = image->height - 1; y1 >= 0; y1--)
        if (!hline_same_alpha(image, y1, 16))
            break;

    for (x0 = 0; x0 < image->width; x0++)
        if (!vline_same_alpha(image, x0, 16))
            break;

    for (x1 = image->width - 1; x1 >= 0; x1--)
        if (!vline_same_alpha(image, x1, 16))
            break;

    if (x1 < x0 || y1 < y0)
        no_clip = 1;

    if (no_clip) {
        x0 = 0;
        y0 = 0;
        x1 = image->width - 1;
        y1 = image->height - 1;
    }

    clip->x0 = x0;
    clip->y0 = y0;
    clip->x1 = x1;
    clip->y1 = y1;
    clip->w = x1 - x0 + 1;
    clip->h = y1 - y0 + 1;
}
