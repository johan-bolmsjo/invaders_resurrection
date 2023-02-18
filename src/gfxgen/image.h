#pragma once

#include <inttypes.h>

#define CMAP 0 /* 8 bit colour mapped image */
#define GREY 1 /* 8 bit grey scale image */
#define RGB  2 /* 24 bit colour image */
#define RGBA 3 /* 32 bit colour image */

typedef struct _Image Image;

struct _Image {
    int type;       /* Image type */
    int colours;    /* Colours in image if type == CMAP 1 to 256 */
    int depth;      /* Bytes per pixel */
    int width;      /* Width of image in pixels */
    int height;     /* Height of image in pixels */
    uint8_t* cmap;  /* Pointer to colour map if type == CMAP */
    uint8_t* image; /* Pointer to image data */
    Image* prev;
    Image* next;
};

Image* image_get_first(void);
Image* image_create(int type, int colours, int width, int height);
void   image_destroy(Image* image);
void   image_destroy_all(void);
void   image_flip_x(Image* im);
void   image_flip_y(Image* im);
