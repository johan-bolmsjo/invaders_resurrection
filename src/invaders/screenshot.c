#include "screenshot.h"
#include "libmedia/libmedia.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

bool
screenshot_create(const struct MLGraphicsBuffer* dst, const char* path)
{
    const uint8_t tga_header[18] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 2, 224, 1, 24, 32};

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        return false;
    }

    uint8_t*  m = malloc(MLDisplayWidth * MLDisplayWidth * 3);
    uint16_t* s = ml_graphics_buffer_xy(dst, 0, 0);
    uint8_t*  d = m;

    for (int i = MLDisplayWidth * MLDisplayHeight; i > 0; i--) {
        const uint16_t sv = *s++;
        /* BGR pixel format */
        *d++ = (sv & 31) << 3;
        *d++ = (sv >> 3) & 0xFC;
        *d++ = (sv >> 8) & 0xF8;
    }

    write(fd, tga_header, 18);
    write(fd, m, MLDisplayWidth * MLDisplayHeight * 3);

    close(fd);
    free(m);
    return true;
}
