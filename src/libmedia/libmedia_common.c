#include "libmedia.h"

#include <stdlib.h>
#include <string.h>

struct MLGraphicsBuffer*
ml_graphics_buffer_create(enum MLPixelFormat format, struct MLRectDim dim)
{
    struct MLGraphicsBuffer* buf = malloc(sizeof(*buf));
    buf->format = format;
    buf->dim = dim;
    buf->pixels = malloc(ml_graphics_buffer_size_bytes(buf));
    ml_graphics_buffer_clear(buf);
    return buf;
}

void
ml_graphics_buffer_destroy(struct MLGraphicsBuffer* buf)
{
    if (buf) {
        free(buf->pixels);
        free(buf);
    }
}

void
ml_graphics_buffer_clear(const struct MLGraphicsBuffer* buf)
{
    memset(buf->pixels, 0, ml_graphics_buffer_size_bytes(buf));
}
