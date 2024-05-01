#include "image.h"

#include <stdlib.h>

static int images = 0;
static struct Image* image_base;

// For other files.
struct Image*
image_get_first(void)
{
    return image_base;
}

// Creates a new image and returns a pointer to this image or NULL
// if there was not enough memory or some input parameter was out of range.
struct Image*
image_create(int type, int colors, int width, int height)
{
    if (width > 32767 ||
        width <= 0 ||
        height > 32767 ||
        height <= 0) {

        return NULL;
    }

    struct Image *image;
    if (!(image = malloc(sizeof(struct Image)))) {
        return NULL;
    }

    image->type = type;
    image->width = width;
    image->height = height;

    switch (type) {
    case CMAP:
        if (colors < 2 || colors > 256) {
            free(image);
            return NULL;
        }
        image->colors = colors;
        image->depth = 1;
        image->cmap = malloc((size_t)colors * 3);
        if (image->cmap == NULL) {
            free(image);
            return NULL;
        }
        break;

    case GREY:
        image->colors = 0;
        image->depth = 1;
        image->cmap = 0;
        break;

    case RGB:
        image->colors = 0;
        image->depth = 3;
        image->cmap = 0;
        break;

    case RGBA:
        image->colors = 0;
        image->depth = 4;
        image->cmap = 0;
        break;

    default:
        free(image);
        return NULL;
    }
    image->image = malloc((size_t)width * height * image->depth);
    if (image->image == NULL) {
        if (image->cmap) {
            free(image->cmap);
        }
        free(image);
        return NULL;
    }

    if (images) {
        struct Image* image2 = image_base;
        while (image2->next) {
            image2 = image2->next;
        }
        image2->next = image;
        image->prev = image2;
    } else {
        image_base = image;
        image->prev = 0;
    }
    image->next = 0;
    images++;

    return image;
}

// Deletes an image.
void
image_destroy(struct Image* image)
{
    if (image->prev) {
        image->prev->next = image->next;
    } else {
        image_base = image->next;
    }
    if (image->next) {
        image->next->prev = image->prev;
    }
    if (image->cmap) {
        free(image->cmap);
    }
    free(image->image);
    free(image);

    images--;
}

// Deletes all images.
void
image_destroy_all(void)
{
    while (image_base) {
        image_destroy(image_base);
    }
}

// Flip image around X-axis
// TODO(jb): Write this one when needed.
void
image_flip_x(struct Image* im)
{
    (void)im;
}

// Flip image around Y-axis
void
image_flip_y(struct Image* im)
{
    const int width = im->width * im->depth;
    uint8_t* top = im->image;
    uint8_t* bot = top + (im->height - 1) * width;

    while (bot > top) {
        for (int i = 0; i < width; i++) {
            const uint8_t tmp = *top;
            *top++ = bot[i];
            bot[i] = tmp;
        }
        bot -= width;
    }
}
