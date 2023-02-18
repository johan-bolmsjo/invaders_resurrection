/* Crop image */

#include <stdlib.h>
#include <inttypes.h>
#include "image.h"
#include "clip.h"

Image*
crop(Image* image, Clip* clip)
{
    int x, y, l, stride;
    char *src, *dst;
    Image* image2;

    if (clip->x0 >= image->width ||
        clip->x1 >= image->width ||
        clip->y0 >= image->height ||
        clip->y1 >= image->height)
        return NULL;

    image2 = image_create(image->type, image->colours, clip->w, clip->h);
    if (!image2)
        return image2;

    src = image->image + image->width * image->depth * clip->y0 +
          clip->x0 * image->depth;
    dst = image2->image;
    l = clip->w * image->depth;
    stride = image->width * image->depth - l;

    for (y = clip->h; y > 0; y--) {
        for (x = l; x > 0; x--)
            *dst++ = *src++;
        src += stride;
    }

    return image2;
}
