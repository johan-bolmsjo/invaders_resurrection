#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "gfx_common.h"

static GfxObject* object_base = 0;

/* For other files.
 */

GfxObject*
gfx_get_first_object()
{
    return object_base;
}

/* Find object with name 'name'.
 */

GfxObject*
gfx_object_find(uint8_t* name)
{
    int l, i;
    GfxObject* o = object_base;

    l = strlen(name);
    while (o) {
        if (l == o->name_len) {
            for (i = 0; i < l; i++)
                if (name[i] != o->name[i])
                    break;
            if (i == l)
                return o;
        }
        o = o->next;
    }

    return NULL;
}

GfxObject*
gfx_object_create(uint8_t* name)
{
    int len;
    uint8_t* n;
    GfxObject* o;

    len = strlen(name);
    if (len > 255)
        return NULL;

    n = malloc(len);
    o = malloc(sizeof(GfxObject));

    if (!n || !o) {
        if (!n)
            free(n);
        if (!o)
            free(o);
        return NULL;
    }

    memcpy(n, name, len);
    o->name_len = len;
    o->name = n;
    o->frames = 0;
    o->fpp = 0;

    if (object_base) {
        o->next = object_base;
        o->next->prev = o;
    } else {
        o->next = 0;
    }
    o->prev = 0;
    object_base = o;

    return o;
}

void
gfx_object_destroy(GfxObject* o)
{
    int i;

    if (o->prev)
        o->prev->next = o->next;
    else
        object_base = o->next;
    if (o->next)
        o->next->prev = o->prev;

    for (i = 0; i < o->frames; i++)
        gfx_frame_destroy(o->fpp[i]);

    free(o->name);
    if (o->fpp)
        free(o->fpp);
    free(o);
}

void
gfx_object_destroy_all()
{
    while (object_base)
        gfx_object_destroy(object_base);
}

GfxFrame*
gfx_frame_create(int flags, int width, int height, int x_off, int y_off)
{
    int c_longs, e = 0;
    void *g = 0, *a = 0, *c = 0;
    GfxFrame* f;

    c_longs = (width + 31) >> 5;
    f = malloc(sizeof(GfxFrame));

    if (flags & GFX_TAG_GRAPHICS) {
        if ((g = malloc(width * height * sizeof(uint16_t))) == NULL)
            e = 1;
    }

    if (flags & GFX_TAG_ALPHA) {
        if ((a = malloc(width * height * sizeof(uint8_t))) == NULL)
            e = 1;
    }

    if (flags & GFX_TAG_COLLISION) {
        if ((c = malloc(c_longs * sizeof(uint32_t) * height)) == NULL)
            e = 1;
    }

    if (e || f == NULL) {
        if (g)
            free(g);
        if (a)
            free(a);
        if (c)
            free(c);
        return NULL;
    }

    f->width = width;
    f->height = height;
    f->x_off = x_off;
    f->y_off = y_off;
    f->c_longs = c_longs;
    f->graphics = g;
    f->alpha = a;
    f->collision = c;

    return f;
}

void
gfx_frame_destroy(GfxFrame* f)
{
    free(f->graphics);
    free(f->alpha);
    free(f->collision);
    free(f);
}

int
gfx_add_frame_to_object(GfxFrame* f, GfxObject* o)
{
    int frames;
    GfxFrame** fpp;

    frames = o->frames + 1;
    fpp = realloc(o->fpp, frames * sizeof(GfxFrame*));
    if (!fpp)
        return -1;

    o->frames = frames;
    o->fpp = fpp;
    fpp[frames - 1] = f;

    return 0;
}
