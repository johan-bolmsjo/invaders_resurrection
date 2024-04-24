/* Partial targa decoder.
 */

#include "ff_targa.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <zlib.h>

#include "error.h"
#include "colours.h"

#define HALF_BUF_SIZE 2048
#define BUF_SIZE      4096
#define BUF_MASK      0xFFF

/* Might screw up the image if fed with faulty rle data.
 * This is acceptable.
 */

static int
rle_read_8(gzFile gz, struct Image* image)
{
    uint8_t buf[BUF_SIZE], *dst = image->image, eof = 0;
    int index = 0, fill = 0, offset = 0;
    int32_t pc = 0;

    const int32_t pixels = (int32_t)image->width * image->height;

    while (pc < pixels) {
        if (fill <= HALF_BUF_SIZE && !eof) {
            int len = gzread(gz, buf + offset, HALF_BUF_SIZE);
            if (len != HALF_BUF_SIZE) {
                if (len == -1) {
                    return E_READ;
                }
                eof = 1;
            }
            fill += len;
            offset ^= HALF_BUF_SIZE;
        }
        uint8_t c = buf[index++];
        index &= BUF_MASK;
        int count = (c & TARGA_RLE_MASK_COUNT);
        pc += count + 1;
        if (pc > pixels) {
            break;
        }
        fill -= 2;
        if (c & TARGA_RLE_MASK_TYPE) {
            c = buf[index++];
            index &= BUF_MASK;
            for (; count >= 0; count--) {
                *dst++ = c;
            }
        } else {
            fill -= count;
            for (; count >= 0; count--) {
                *dst++ = buf[index++];
                index &= BUF_MASK;
            }
        }
    }

    if (pc != pixels) {
        return ERROR;
    }

    return E_OK;
}

static int
rle_read_24(gzFile gz, struct Image* image)
{
    uint8_t buf[BUF_SIZE], *dst = image->image, eof = 0;
    int index = 0, fill = 0, offset = 0;
    int32_t pc = 0;

    int32_t pixels = (int32_t)image->width * image->height;

    while (pc < pixels) {
        if (fill <= HALF_BUF_SIZE && !eof) {
            int len = gzread(gz, buf + offset, HALF_BUF_SIZE);
            if (len != HALF_BUF_SIZE) {
                if (len == -1) {
                    return E_READ;
                }
                eof = 1;
            }
            fill += len;
            offset ^= HALF_BUF_SIZE;
        }
        const uint8_t c = buf[index++];
        index &= BUF_MASK;
        int count = (c & TARGA_RLE_MASK_COUNT);
        pc += count + 1;
        if (pc > pixels) {
            break;
        }
        fill -= 4;
        if (c & TARGA_RLE_MASK_TYPE) {
            const uint8_t r = buf[index++];
            index &= BUF_MASK;
            const uint8_t g = buf[index++];
            index &= BUF_MASK;
            const uint8_t b = buf[index++];
            index &= BUF_MASK;
            for (; count >= 0; count--) {
                *dst++ = r;
                *dst++ = g;
                *dst++ = b;
            }
        } else {
            fill -= count * 3;
            for (; count >= 0; count--) {
                *dst++ = buf[index++];
                index &= BUF_MASK;
                *dst++ = buf[index++];
                index &= BUF_MASK;
                *dst++ = buf[index++];
                index &= BUF_MASK;
            }
        }
    }

    if (pc != pixels) {
        return ERROR;
    }

    return E_OK;
}

struct Image*
ff_targa_read(char* path)
{
    uint8_t buf[18];
    struct TargaHeader tga;
    struct Image* image = NULL;

    gzFile gz = gzopen(path, "rb");
    if (!gz) {
        return NULL;
    }

    if (gzread(gz, buf, 18) != 18) {
        gzclose(gz);
        return NULL;
    }

    tga.id_length = buf[0];
    tga.cmap_type = buf[1];
    tga.image_type = buf[2];
    tga.cmap_spec.colours = ((int)buf[6] << 8) | buf[5];
    tga.cmap_spec.entry_size = buf[7];
    tga.image_spec.width = ((int)buf[13] << 8) | buf[12];
    tga.image_spec.height = ((int)buf[15] << 8) | buf[14];
    tga.image_spec.depth = buf[16];
    tga.image_spec.descriptor = buf[17];

    if (tga.id_length) {
        if (gzseek(gz, tga.id_length, SEEK_CUR) != (18 + tga.id_length)) {
            gzclose(gz);
            return NULL;
        }
    }

    if (tga.cmap_type != TARGA_CMAP_NO && tga.cmap_type != TARGA_CMAP_YES) {
        gzclose(gz);
        return NULL;
    }

    switch (tga.image_type & ~TARGA_TYPE_RLE_MASK) {
    case TARGA_TYPE_CMAP:
        if (tga.image_spec.depth != 8) {
            break;
        }
        if (tga.cmap_type == TARGA_CMAP_NO) {
            break;
        }
        if (tga.cmap_spec.entry_size != 24) {
            break;
        }

        image = image_create(CMAP, tga.cmap_spec.colours,
                             tga.image_spec.width, tga.image_spec.height);
        if (image == NULL) {
            break;
        }

        int len = tga.cmap_spec.colours * ((tga.cmap_spec.entry_size + 7) >> 3);
        if (gzread(gz, image->cmap, len) != len) {
            gzclose(gz);
            return NULL;
        }

        if (tga.image_type & TARGA_TYPE_RLE_MASK) {
            if (rle_read_8(gz, image) == ERROR) {
                image_destroy(image);
                image = NULL;
            }
        } else {
            int len = (size_t)image->width * image->height;
            if (gzread(gz, image->image, len) != len) {
                image_destroy(image);
                image = NULL;
            }
        }
        break;

    case TARGA_TYPE_RGB:
        if (tga.cmap_type == TARGA_CMAP_YES) {
            int len = tga.cmap_spec.colours * ((tga.cmap_spec.entry_size + 7) >> 3);
            if (gzseek(gz, len, SEEK_CUR) != len) {
                gzclose(gz);
                return NULL;
            }
        }
        if (tga.image_spec.depth != 24) {
            break;
        }

        image = image_create(RGB, 0, tga.image_spec.width,
                             tga.image_spec.height);
        if (image == NULL) {
            break;
        }

        if (tga.image_type & TARGA_TYPE_RLE_MASK) {
            if (rle_read_24(gz, image) == ERROR) {
                image_destroy(image);
                image = NULL;
            }
        } else {
            int len = (size_t)image->width * image->height * 3;
            if (gzread(gz, image->image, len) != len) {
                image_destroy(image);
                image = NULL;
            }
        }
        /* POV-Ray stores targa images in bgr order */
        if (image) {
            colour_bgr_to_rgb(image);
        }
        break;

    case TARGA_TYPE_GREY:
        if (tga.cmap_type == TARGA_CMAP_YES) {
            int len = tga.cmap_spec.colours * ((tga.cmap_spec.entry_size + 7) >> 3);
            if (gzseek(gz, len, SEEK_CUR) != len) {
                gzclose(gz);
                return NULL;
            }
        }
        if (tga.image_spec.depth != 8) {
            break;
        }

        image = image_create(GREY, 0, tga.image_spec.width,
                             tga.image_spec.height);
        if (image == NULL) {
            break;
        }

        if (tga.image_type & TARGA_TYPE_RLE_MASK) {
            if (rle_read_8(gz, image) == ERROR) {
                image_destroy(image);
                image = NULL;
            }
        } else {
            int len = (size_t)image->width * image->height;
            if (gzread(gz, image->image, len) != len) {
                image_destroy(image);
                image = NULL;
            }
        }
    }

    if (!(tga.image_spec.descriptor & TARGA_VERT_MASK)) {
        image_flip_y(image);
    }

    gzclose(gz);
    return image;
}
