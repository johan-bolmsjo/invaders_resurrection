/* Create RGB targa snap-shot of the visible screen.
 */

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include "all.h"

int
snap_create(DG* dg, char* path)
{
    int fd, i;
    uint16_t *s, sv;
    uint8_t *m, *d, targa_header[18] = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 2, 224, 1, 24, 32};

    fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1)
        return E_OPEN;

    m = malloc(DG_XRES * DG_XRES * 3);
    if (m == NULL)
        return E_MEM;

    s = dg->adr[dg->vis];
    d = m;

    for (i = DG_XRES * DG_YRES; i > 0; i--) {
        sv = *s++;
        /* BGR */
        *d++ = (sv & 31) << 3;
        *d++ = (sv >> 3) & 0xFC;
        *d++ = (sv >> 8) & 0xF8;
    }

    write(fd, targa_header, 18);
    write(fd, m, DG_XRES * DG_YRES * 3);

    close(fd);
    free(m);
    return E_OK;
}
