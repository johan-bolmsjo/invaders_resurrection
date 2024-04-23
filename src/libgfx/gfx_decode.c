#include "gfx_decode.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "endian.h"
#include "gfx_object.h"
#include "libutil/array.h"
#include "libutil/endian.h"

static bool
xreturn(gzFile gz, struct GfxObject* o, struct GfxFrame* f, bool rval)
{
    if (gz) {
        gzclose(gz);
    }
    if (o) {
        gfx_object_destroy(o);
    }
    if (f) {
        gfx_frame_destroy(f);
    }
    return rval;
}

bool
gfx_decode(const uint8_t* src, size_t len)
{
    int n;
    uint8_t tag, name_len;
    char name[256];
    uint16_t sa[4];
    struct GfxObject* o = 0;
    struct GfxFrame* f;

    const uint8_t* end = src + len;

    while (src < end) {
        tag = *src++;
        switch (tag & 0xF0) {
        case GFX_TAG_OBJECT:
            name_len = *src++;
            memcpy(name, src, name_len);
            name[name_len] = 0;
            src += name_len;

            o = gfx_object_create(name);
            if (o == NULL) {
                return xreturn(NULL, o, NULL, false);
            }
            break;

        case GFX_TAG_FRAME:
            if (!o) {
                return xreturn(NULL, o, NULL, false);
            }

            memcpy(sa, src, 8);
            src += 8;
            ntohs_n(sa, ARRAY_SIZE(sa));

            f = gfx_frame_create(tag, sa[0], sa[1], sa[2], sa[3]);
            if (f == NULL) {
                return xreturn(NULL, o, NULL, false);
            }

            n = f->width * f->height;

            if (tag & GFX_TAG_GRAPHICS) {
                memcpy(f->graphics, src, n * 2);
                src += (n * 2);
                ntohs_n(&f->graphics[0].v, n);
            }

            if (tag & GFX_TAG_ALPHA) {
                memcpy(f->alpha, src, n);
                src += n;
            }

            if (tag & GFX_TAG_COLLISION) {
                n = f->c_longs * f->height;
                memcpy(f->collision, src, n * 4);
                src += (n * 4);
                ntohl_n(f->collision, n);
            }

            if (gfx_add_frame_to_object(f, o)) {
                return xreturn(NULL, o, f, false);
            }
            break;

        default:
            return false;
        }
    }

    return true;
}
