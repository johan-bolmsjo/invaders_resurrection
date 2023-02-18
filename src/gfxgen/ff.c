#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <ctype.h>
#include <inttypes.h>
#include "image.h"
#include "ff.h"
#include "ff_targa.h"
#include "error.h"

static int
streq(char* s1, char* s2, int len)
{
    int i;

    for (i = 0; i < len; i++) {
        if (tolower(s1[i]) != tolower(s2[i]))
            return FALSE;
    }

    return TRUE;
}

static int
match_extention(FileFormat* formats, char* path)
{
    int i = 0, j, pl, el;

    pl = strlen(path);

    while (formats[i].func) {
        j = 0;
        while (formats[i].exts[j]) {
            el = strlen(formats[i].exts[j]);
            if (el <= pl) {
                if (streq(formats[i].exts[j], path + pl - el, el))
                    return i;
            }
            j++;
        }
        i++;
    }

    return ERROR;
}

/* This function reads all supported image types.
 * Returns an Image pointer on success and NULL on failure.
 */

Image*
ff_read(char* path)
{
    int i, try;
    char* targa[] = {".tga", ".tga.gz", 0};
    Image* image;
    FileFormat formats[] = {{targa, ff_targa_read},
                            {0, 0}};

    try = match_extention(formats, path);
    if (try == ERROR)
        try = 0;

    image = formats[try].func(path);
    if (image == NULL) {
        i = 0;
        while (formats[i].func) {
            if (i != try) {
                image = formats[i].func(path);
                if (image)
                    break;
            }
            i++;
        }
    }

    return image;
}
