#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>
#include "gfx_common.h"
#include "msb.h"

static int
gfx_read_exit(gzFile gz, GfxObject* o, GfxFrame* f, int error_code)
{
    if (gz)
        gzclose(gz);
    if (o)
        gfx_object_destroy(o);
    if (f)
        gfx_frame_destroy(f);

    return error_code;
}

int
gfx_read(uint8_t* path)
{
    int r = 0, r2 = 0, n;
    uint8_t tag, name[256], name_len;
    int16_t sa[4];
    GfxObject* o = 0;
    GfxFrame* f;
    gzFile gz;

    gz = gzopen(path, "rb");
    if (gz == NULL)
        return gfx_read_exit(gz, 0, 0, -1);

    while (gzread(gz, &tag, 1)) {
        r += 1;
        r2 += 1;
        switch (tag & 0xF0) {
        case GFX_TAG_OBJECT:
            r += gzread(gz, &name_len, 1);
            r += gzread(gz, name, name_len);
            name[name_len] = 0;
            o = gfx_object_create(name);
            if (o == NULL)
                return gfx_read_exit(gz, o, 0, -1);

            r2 += 1 + name_len;
            if (r != r2)
                return gfx_read_exit(gz, o, 0, -1);
            break;

        case GFX_TAG_FRAME:
            if (!o)
                return gfx_read_exit(gz, o, 0, -1);

            r += gzread(gz, sa, 8);
            r2 += 8;
            if (r != r2)
                return gfx_read_exit(gz, o, 0, -1);

            msb_short_mem(sa, 4);
            f = gfx_frame_create(tag, sa[0], sa[1], sa[2], sa[3]);
            if (f == NULL)
                return gfx_read_exit(gz, o, 0, -1);

            n = f->width * f->height;

            if (tag & GFX_TAG_GRAPHICS) {
                r += gzread(gz, f->graphics, n * 2);
                r2 += n * 2;
                if (r != r2)
                    return gfx_read_exit(gz, o, f, -1);
                msb_short_mem(f->graphics, n);
            }

            if (tag & GFX_TAG_ALPHA) {
                r += gzread(gz, f->alpha, n);
                r2 += n;
                if (r != r2)
                    return gfx_read_exit(gz, o, f, -1);
            }

            if (tag & GFX_TAG_COLLISION) {
                n = f->c_longs * f->height;
                r += gzread(gz, f->collision, n * 4);
                r2 += n * 4;
                if (r != r2)
                    return gfx_read_exit(gz, o, f, -1);
                msb_long_mem(f->collision, n);
            }

            if (gfx_add_frame_to_object(f, o))
                return gfx_read_exit(gz, o, f, -1);
            break;

        default:
            return gfx_read_exit(gz, 0, 0, -1);
        }
    }

    return 0;
}

int
gfx_decode(uint8_t* src, int len)
{
    int n;
    uint8_t tag, name[256], name_len, *end;
    int16_t sa[4];
    GfxObject* o = 0;
    GfxFrame* f;

    end = src + len;

    while (src < end) {
        tag = *src++;
        switch (tag & 0xF0) {
        case GFX_TAG_OBJECT:
            name_len = *src++;
            memcpy(name, src, name_len);
            name[name_len] = 0;
            src += name_len;

            o = gfx_object_create(name);
            if (o == NULL)
                return gfx_read_exit(0, o, 0, -1);
            break;

        case GFX_TAG_FRAME:
            if (!o)
                return gfx_read_exit(0, o, 0, -1);

            memcpy(sa, src, 8);
            src += 8;
            msb_short_mem(sa, 4);

            f = gfx_frame_create(tag, sa[0], sa[1], sa[2], sa[3]);
            if (f == NULL)
                return gfx_read_exit(0, o, 0, -1);

            n = f->width * f->height;

            if (tag & GFX_TAG_GRAPHICS) {
                memcpy(f->graphics, src, n * 2);
                src += (n * 2);
                msb_short_mem(f->graphics, n);
            }

            if (tag & GFX_TAG_ALPHA) {
                memcpy(f->alpha, src, n);
                src += n;
            }

            if (tag & GFX_TAG_COLLISION) {
                n = f->c_longs * f->height;
                memcpy(f->collision, src, n * 4);
                src += (n * 4);
                msb_long_mem(f->collision, n);
            }

            if (gfx_add_frame_to_object(f, o))
                return gfx_read_exit(0, o, f, -1);
            break;

        default:
            return gfx_read_exit(0, 0, 0, -1);
        }
    }

    return 0;
}
