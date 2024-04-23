#include "gfx_object.h"

#include <stdlib.h>
#include <string.h>

static struct GfxObject* object_list = 0;

/* For other files.
 */

struct GfxObject*
gfx_get_first_object(void)
{
    return object_list;
}

/* Find object with name 'name'.
 */

struct GfxObject*
gfx_object_find(const char* name)
{
    int l, i;
    struct GfxObject* o = object_list;

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

struct GfxObject*
gfx_object_create(const char* name)
{
    const int name_len = strlen(name);
    if (name_len > 255)
        return NULL;

    char* name_copy = strdup(name);
    struct GfxObject* obj = malloc(sizeof(struct GfxObject));

    if (!name_copy || !obj) {
        free(name_copy);
        free(obj);
        return NULL;
    }

    obj->name_len = name_len;
    obj->name = name_copy;
    obj->frames = 0;
    obj->fpp = 0;

    if (object_list) {
        obj->next = object_list;
        obj->next->prev = obj;
    } else {
        obj->next = 0;
    }
    obj->prev = 0;
    object_list = obj;

    return obj;
}

void
gfx_object_destroy(struct GfxObject* o)
{
    int i;

    if (o->prev)
        o->prev->next = o->next;
    else
        object_list = o->next;
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
gfx_object_destroy_all(void)
{
    while (object_list)
        gfx_object_destroy(object_list);
}

struct GfxFrame*
gfx_frame_create(int flags, int width, int height, int x_off, int y_off)
{
    int c_longs, e = 0;
    void *g = 0, *a = 0, *c = 0;
    struct GfxFrame* f;

    c_longs = (width + 31) >> 5;
    f = malloc(sizeof(struct GfxFrame));

    if (flags & GFX_TAG_GRAPHICS) {
        if ((g = malloc(width * height * sizeof(struct rgb565))) == NULL)
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
gfx_frame_destroy(struct GfxFrame* f)
{
    free(f->graphics);
    free(f->alpha);
    free(f->collision);
    free(f);
}

int
gfx_add_frame_to_object(struct GfxFrame* f, struct GfxObject* o)
{
    int frames;
    struct GfxFrame** fpp;

    frames = o->frames + 1;
    fpp = realloc(o->fpp, frames * sizeof(struct GfxFrame*));
    if (!fpp)
        return -1;

    o->frames = frames;
    o->fpp = fpp;
    fpp[frames - 1] = f;

    return 0;
}
