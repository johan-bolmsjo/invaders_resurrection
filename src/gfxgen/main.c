#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "libgfx/libgfx.h"
#include "image.h"
#include "clip.h"
#include "error.h"
#include "def.h"
#include "gfx_write.h"

int
main(int argc, char** argv)
{
    int8_t c;
    GfxObject* o;

    extern char* optarg;
    extern int optind, opterr, optopt;

    while ((c = getopt(argc, argv, "hi:o:")) != -1) {
        switch (c) {
        case 'h':
            printf("usage: gfxgen -i <input def file> -o <output gfx file>\n");
            return E_OK;
            break;

        case 'i':
            if (def_run(optarg)) {
                printf("ERROR\n");
                return 2;
            }
            break;

        case 'o':
            o = gfx_get_first_object();
            if (o) {
                if (gfx_write(o, optarg))
                    return 2;
            }
            break;

        case ':':
            fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            return 1;

        case '?':
            fprintf(stderr, "Unrecognized option: -%c.\n", optopt);
            return 1;
        }
    }

    return E_OK;
}
