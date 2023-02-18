#include <stdlib.h>
#include <netinet/in.h>
#include <zlib.h>
#include "all.h"
#include "gfx_data.c"

int
gfx_decode2()
{
    Bytef* dst_buf;
    uLong src_len, dst_len;

    src_len = ntohl(*(uint32_t*)(gfx_data + 4));
    dst_len = ntohl(*(uint32_t*)gfx_data);

    if (!(dst_buf = malloc(dst_len)))
        return 1;

    if (uncompress(dst_buf, &dst_len, gfx_data + 8, src_len)) {
        free(dst_buf);
        return 1;
    }

    if (gfx_decode(dst_buf, dst_len)) {
        free(dst_buf);
        return 1;
    }

    free(dst_buf);

    return 0;
}
