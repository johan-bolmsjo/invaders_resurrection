#include "colors.h"

// bgr to rgb, fix targa code later instead of this hack!
void
color_bgr_to_rgb(struct Image* image)
{
    uint8_t* p = image->image;
    for (int i = image->width * image->height; i > 0; i--) {
        uint8_t c = p[0];
        p[0] = p[2];
        p[2] = c;
        p += 3;
    }
}

// Add an alpha channel to a rgb image, store as rgba.
struct Image*
color_rgb_to_rgba(struct Image* image)
{
    struct Image* image2 = image_create(RGBA, 0, image->width, image->height);
    if (!image2) {
        return image2;
    }

    uint8_t* s = image->image;
    uint8_t* d = image2->image;

    for (int i = image->width * image->height; i > 0; i--) {
        *d++ = *s++;
        *d++ = *s++;
        *d++ = *s++;
        *d++ = 0;
    }

    return image2;
}

// Mix each pixel with its alpha channel and black, store as rgb.
struct Image*
color_rgba_to_rgb(struct Image* image)
{
    struct Image* image2 = image_create(RGB, 0, image->width, image->height);
    if (!image2) {
        return image2;
    }

    uint8_t* s = image->image;
    uint8_t* d = image2->image;

    for (int i = image->width * image->height; i > 0; i--) {
        int a = (16 - s[3]) << 4;
        *d++ = (s[0] * a) >> 8;
        *d++ = (s[1] * a) >> 8;
        *d++ = (s[2] * a) >> 8;
        s += 4;
    }

    return image2;
}

// Game specific alpha.
void
color_make_rgb_transparent(struct Image* image, uint8_t* rgb, uint8_t* thres)
{
    if (image->type != RGBA) {
        return;
    }

    uint8_t* p = image->image;
    for (int i = image->width * image->height; i > 0; i--) {
        int j;
        for (j = 0; j < 3; j++) {
            int min = (int)rgb[j] - thres[j];
            if (min < 0) {
                min = 0;
            }
            int max = (int)rgb[j] + thres[j];
            if (max > 255) {
                max = 255;
            }
            if (p[j] < min || p[j] > max) {
                break;
            }
        }
        if (j == 3) {
            p[j] = 16;
        }
        p += 4;
    }
}
