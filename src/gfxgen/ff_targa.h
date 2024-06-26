#pragma once

#include <inttypes.h>

#include "image.h"

// Color map types
#define TARGA_CMAP_NO  0
#define TARGA_CMAP_YES 1

// Image types
#define TARGA_TYPE_CMAP     1
#define TARGA_TYPE_RGB      2
#define TARGA_TYPE_GREY     3
#define TARGA_TYPE_RLE_MASK 8

// Image descriptor
#define TARGA_ALPHA_MASK 15 // Extention bits of pixel depth
#define TARGA_HORIZ_MASK 16 // Image direction
#define TARGA_VERT_MASK  32

// RLE
#define TARGA_RLE_MASK_TYPE  128 // 1 = RLE packet, 0 = RAW packet
#define TARGA_RLE_MASK_COUNT 127

struct TargaCmapSpec {
    // int first_entry;
    int     colors;
    uint8_t entry_size;         // Typically 15, 16, 24 or 32
};

struct TargaImageSpec {
    // int x_origin;
    // int y_origin;
    int     width;
    int     height;
    uint8_t depth;              // Typically 8, 16, 24 or 32
    uint8_t descriptor;
};

struct TargaHeader {
    uint8_t               id_length;
    uint8_t               cmap_type;
    uint8_t               image_type;
    struct TargaCmapSpec  cmap_spec;
    struct TargaImageSpec image_spec;
};

struct Image* ff_targa_read(char* path);
