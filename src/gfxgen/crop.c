#include "crop.h"

#include <stddef.h>

struct Image*
crop(struct Image* image, struct Clip* clip)
{
    if (clip->x0 >= image->width ||
        clip->x1 >= image->width ||
        clip->y0 >= image->height ||
        clip->y1 >= image->height) {

        return NULL;
    }

    struct Image* image2 = image_create(image->type, image->colours, clip->w, clip->h);
    if (!image2) {
        return image2;
    }

    uint8_t* src = image->image + image->width * image->depth * clip->y0 + clip->x0 * image->depth;
    uint8_t* dst = image2->image;
    int l = clip->w * image->depth;
    int stride = image->width * image->depth - l;

    for (int y = clip->h; y > 0; y--) {
        for (int x = l; x > 0; x--) {
            *dst++ = *src++;
        }
        src += stride;
    }

    return image2;
}
