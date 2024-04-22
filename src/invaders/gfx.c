#include "gfx.h"

#include <stdlib.h>
#include <netinet/in.h>
#include <zlib.h>

#include "compressed_gfx_data.c"
#include "libgfx/libgfx.h"
#include "libutil/endian.h"

bool
decode_gfx_data(void)
{
    Bytef* dst_buf;
    uLong src_len, dst_len;

    src_len = ntohl(*(uint32_t*)(compressed_gfx_data + 4));
    dst_len = ntohl(*(uint32_t*)compressed_gfx_data);

    if (!(dst_buf = malloc(dst_len)))
        return false;

    if (uncompress(dst_buf, &dst_len, compressed_gfx_data + 8, src_len) != Z_OK) {
        free(dst_buf);
        return false;
    }

    if (!gfx_decode(dst_buf, dst_len)) {
        free(dst_buf);
        return false;
    }

    free(dst_buf);
    return true;
}
