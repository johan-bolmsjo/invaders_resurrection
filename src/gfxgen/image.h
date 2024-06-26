#pragma once

#include <inttypes.h>

#define CMAP 0  // 8 bit color mapped image
#define GREY 1  // 8 bit grey scale image
#define RGB  2  // 24 bit color image
#define RGBA 3  // 32 bit color image

struct Image {
    int           type;         // Image type
    int           colors;       // Colors in image if type == CMAP 1 to 256
    int           depth;        // Bytes per pixel
    int           width;        // Width of image in pixels
    int           height;       // Height of image in pixels
    uint8_t*      cmap;         // Pointer to color map if type == CMAP
    uint8_t*      image;        // Pointer to image data
    struct Image* prev;
    struct Image* next;
};

struct Image* image_get_first(void);
struct Image* image_create(int type, int colors, int width, int height);
void          image_destroy(struct Image* image);
void          image_destroy_all(void);
void          image_flip_x(struct Image* im);
void          image_flip_y(struct Image* im);
