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
streq(const char* s1, const char* s2, int len)
{
    for (int i = 0; i < len; i++) {
        if (tolower(s1[i]) != tolower(s2[i])) {
            return FALSE;
        }
    }
    return TRUE;
}

static int
match_extention(struct FileFormat* formats, char* path)
{
    const int pl = strlen(path);

    int i = 0;
    while (formats[i].func) {
        int j = 0;
        while (formats[i].exts[j]) {
            const int el = strlen(formats[i].exts[j]);
            if (el <= pl) {
                if (streq(formats[i].exts[j], path + pl - el, el)) {
                    return i;
                }
            }
            j++;
        }
        i++;
    }

    return ERROR;
}

// This function reads all supported image types.
// Returns an Image pointer on success and NULL on failure.
struct Image*
ff_read(char* path)
{
    const char* targa[] = {".tga", ".tga.gz", 0};
    struct FileFormat formats[] = {{targa, ff_targa_read},
                                   {0, 0}};

    int try = match_extention(formats, path);
    if (try == ERROR) {
        try = 0;
    }

    struct Image* image = formats[try].func(path);
    if (image == NULL) {
        int i = 0;
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
