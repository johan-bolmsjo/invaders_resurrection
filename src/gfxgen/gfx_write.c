#include "gfx_write.h"

#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>

#include "error.h"
#include "image.h"
#include "libutil/endian.h"

// Create frame graphics from image.
int
gfx_create_graphics(struct GfxFrame* f, struct Image* im)
{
    if (im->type != RGBA && im->type != RGB) {
        return ERROR;
    }

    if (im->width != f->width || im->height != f->height || f->graphics == NULL) {
        return ERROR;
    }

    const int pixels = f->width * f->height;
    uint8_t* s = im->image;
    struct rgb565* d = f->graphics;

    for (int i = 0; i < pixels; i++) {
        d[i] = (struct rgb565){((int)s[0] >> 3) << 11 | ((int)s[1] >> 2) << 5 | (int)s[2] >> 3};
        s += im->depth;
    }

    return E_OK;
}

// Create frame alpha channel mask from image.
int
gfx_create_alpha(struct GfxFrame* f, struct Image* im)
{
    if (im->type != RGBA ||
        im->width != f->width ||
        im->height != f->height ||
        f->alpha == NULL) {
        return ERROR;
    }

    uint8_t* s = im->image + 3;
    uint8_t* d = f->alpha;
    int32_t pixels = f->width * f->height;

    for (; pixels > 0; pixels--) {
        *d++ = *s;
        s += 4;
    }

    return E_OK;
}

// Create collision mask from alpha channel in image.
int
gfx_create_collision(struct GfxFrame* f, struct Image* im)
{
    int i, j, shift;
    uint8_t* s;
    uint32_t block, *d;

    if (im->type != RGBA ||
        im->width != f->width ||
        im->height != f->height ||
        f->collision == NULL)
        return ERROR;

    s = im->image + 3;
    d = f->collision;
    block = 0;
    shift = 31;

    for (i = f->height; i > 0; i--) {
        for (j = f->width - 1; j >= 0; j--) {
            if (*s != 16)
                block |= (int32_t)1 << shift;
            s += im->depth;
            shift--;
            if (shift < 0 || !j) {
                *d++ = block;
                block = 0;
                shift = 31;
            }
        }
    }

    return E_OK;
}

// Writes gfx data to file.
int
gfx_write(struct GfxObject* o, char* path)
{
    gzFile gz = gzopen(path, "wb9");
    if (gz == NULL) {
        return E_OPEN;
    }

    int written = 0, written2 = 0;

    uint8_t otag[2] = {GFX_TAG_OBJECT};
    while (o) {
        otag[1] = o->name_len;
        written2 += 2 + o->name_len;
        written += gzwrite(gz, otag, 2);
        written += gzwrite(gz, o->name, o->name_len);

        for (int frame = 0; frame < o->frames; frame++) {
            struct GfxFrame* fp = o->fpp[frame];
            uint8_t ftag = GFX_TAG_FRAME;

            if (fp->graphics) {
                ftag |= GFX_TAG_GRAPHICS;
            }
            if (fp->alpha) {
                ftag |= GFX_TAG_ALPHA;
            }
            if (fp->collision) {
                ftag |= GFX_TAG_COLLISION;
            }

            uint16_t sa[4];
            sa[0] = htons(fp->width);
            sa[1] = htons(fp->height);
            sa[2] = htons(fp->x_off);
            sa[3] = htons(fp->y_off);
            written2 += 9;
            written += gzwrite(gz, &ftag, 1);
            written += gzwrite(gz, sa, 8);

            struct rgb565* sp = fp->graphics;
            if (sp) {
                int i = fp->width * fp->height;
                written2 += i * 2;
                for (; i > 0; i--) {
                    uint16_t s = htons((*sp++).v);
                    written += gzwrite(gz, &s, 2);
                }
            }

            if (fp->alpha) {
                int i = fp->width * fp->height;
                written2 += i;
                written += gzwrite(gz, fp->alpha, i);
            }

            uint32_t* lp = fp->collision;
            if (lp) {
                int i = fp->c_longs * fp->height;
                written2 += i * 4;
                for (; i > 0; i--) {
                    uint32_t l = htonl(*lp++);
                    written += gzwrite(gz, &l, 4);
                }
            }
        }

        o = o->next;
    }

    gzclose(gz);

    if (written == written2) {
        return E_OK;
    } else {
        return E_WRITE;
    }
}
