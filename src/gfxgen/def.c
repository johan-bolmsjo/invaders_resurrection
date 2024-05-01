// Graphics definition file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "libgfx/libgfx.h"
#include "image.h"
#include "ff.h"
#include "colors.h"
#include "clip.h"
#include "crop.h"
#include "scale.h"
#include "def.h"
#include "gfx_write.h"
#include "error.h"

static struct DefFile d_file;  // En liten fuuuling:)

// Skip line.
static void
skip_line(void)
{
    for (; d_file.pos < d_file.size; d_file.pos++) {
        if (d_file.mem[d_file.pos] == 10)
            break;
    }
}

// Read file to memory
static int
read_file(const char* filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return E_OPEN;
    }

    int size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char* m = malloc(size);
    if (m == NULL) {
        close(fd);
        return E_MEM;
    }

    if (read(fd, m, size) != size) {
        close(fd);
        free(m);
        return E_READ;
    }

    close(fd);

    d_file.mem = m;
    d_file.size = size;
    d_file.pos = 0;

    return E_OK;
}

// Fetch next command, word.
static char*
next_cmd(void)
{
    static char buf[DEF_MAX_CMD_SIZE];

    for (; d_file.pos < d_file.size; d_file.pos++) {
        char c = d_file.mem[d_file.pos];
        if (c == 9 || c == 32 || c == 10) {
            continue;
        }

        if (c == '#') {
            skip_line();
            continue;
        }

        break;
    }

    if (d_file.pos == d_file.size) {
        return NULL;
    }

    int pos = 0;
    for (; d_file.pos < d_file.size; d_file.pos++) {
        if (pos >= DEF_MAX_CMD_SIZE) {
            break;
        }

        char c = d_file.mem[d_file.pos];
        if (c == 9 || c == 32 || c == 10) {
            break;
        }

        buf[pos++] = c;
    }

    buf[pos] = 0;
    return buf;
}

// Create a new object with the given name.
static int
cmd_object(struct DefObject* d_obj)
{
    char* p = next_cmd();
    if (p == NULL) {
        return ERROR;
    }

    printf("object: %s\n", p);

    struct GfxObject* o = gfx_object_create(p);
    if (o == NULL) {
        return ERROR;
    }

    d_obj->o      = o;
    d_obj->frames = 0;
    d_obj->ipp    = NULL;
    d_obj->x_off  = 0;
    d_obj->y_off  = 0;

    return E_OK;
}

// Loads an image into d_obj, and converts it to rgba.
static int
cmd_frame(struct DefObject* d_obj)
{
    char* p = next_cmd();
    if (p == NULL) {
        return ERROR;
    }

    printf("frame: %s\n", p);

    int frames = d_obj->frames + 1;
    struct Image ** ipp2 = realloc(d_obj->ipp, frames * sizeof(struct Image**));
    if (ipp2 == NULL) {
        return E_MEM;
    }

    d_obj->ipp = ipp2;

    struct Image* im = ff_read(p);
    if (im == NULL) {
        return ERROR;
    }

    struct Image* im2 = color_rgb_to_rgba(im);
    image_destroy(im);
    if (im2 == NULL) {
        return ERROR;
    }

    d_obj->frames = frames;
    ipp2[frames - 1] = im2;

    return E_OK;
}

// Converts a two digit ascii hex-code to an integer.
static int
ascii_hex_to_int(char* p)
{
    int v = 0;

    for (int i = 1; i >= 0; i--) {
        char c = *p++;
        if (c >= '0' && c <= '9') {
            c -= '0';
        } else {
            if (c >= 'A' && c <= 'F') {
                c = c - 'A' + 10;
            } else {
                return -1;
            }
        }
        v = v << 4 | c;
    }
    return v;
}

// Adds alpha value 16 to pixels with the specified rgb color.
static int
cmd_rgb_to_alpha(struct DefObject* d_obj)
{
    printf("rgb_to_alpha...\n");

    char* p = next_cmd();
    if (p == NULL) {
        return ERROR;
    }

    if (strlen(p) != 6) {
        return ERROR;
    }

    uint8_t rgb[3];
    for (int i = 0; i < 3; i++) {
        int v = ascii_hex_to_int(p + i * 2);
        if (v < 0) {
            return ERROR;
        }
        rgb[i] = v;
    }

    uint8_t thres[3] = {0, 0, 0};
    for (int i = 0; i < d_obj->frames; i++) {
        color_make_rgb_transparent(d_obj->ipp[i], rgb, thres);
    }

    return E_OK;
}

// Auto crops all images in d_obj.
static int
cmd_autocrop_alpha(struct DefObject* d_obj)
{
    printf("autocrop_alpha...\n");

    for (int i = 0; i < d_obj->frames; i++) {
        if (i == 0) {
            clip_auto_alpha(d_obj->ipp[i], &d_obj->clip);
        } else {
            struct Clip clip;
            clip_auto_alpha(d_obj->ipp[i], &clip);
            if (clip.x0 < d_obj->clip.x0) {
                d_obj->clip.x0 = clip.x0;
            }
            if (clip.y0 < d_obj->clip.y0) {
                d_obj->clip.y0 = clip.y0;
            }
            if (clip.x1 > d_obj->clip.x1) {
                d_obj->clip.x1 = clip.x1;
            }
            if (clip.y1 > d_obj->clip.y1) {
                d_obj->clip.y1 = clip.y1;
            }
        }
    }

    d_obj->clip.w = d_obj->clip.x1 - d_obj->clip.x0 + 1;
    d_obj->clip.h = d_obj->clip.y1 - d_obj->clip.y0 + 1;

    for (int i = 0; i < d_obj->frames; i++) {
        struct Image* image = crop(d_obj->ipp[i], &d_obj->clip);
        if (image == NULL) {
            return ERROR;
        }

        image_destroy(d_obj->ipp[i]);
        d_obj->ipp[i] = image;
    }

    return E_OK;
}

// Converts an ascii integer to a real one.
static int
ascii_to_int(char* p)
{
    int v = 0;

    char c;
    while ((c = *p++)) {
        if (c < '0' || c > '9') {
            return -1;
        }
        v = v * 10 + c - '0';
    }
    return v;
}

// Scales all images in d_obj to fit in the specified window size, keeps
// the aspect ratio.
static int
cmd_scale_maxspect(struct DefObject* d_obj)
{
    printf("scale_maxspect...\n");

    int xy[2];
    for (int i = 0; i < 2; i++) {
        char* p = next_cmd();
        if (p == NULL) {
            return ERROR;
        }

        xy[i] = ascii_to_int(p);
        if (xy[i] < 0) {
            return ERROR;
        }
    }

    if (d_obj->clip.w != d_obj->clip.h) {
        double aspx = (double)d_obj->clip.w / xy[0];
        double aspy = (double)d_obj->clip.h / xy[1];
        if (aspx > aspy) {
            xy[1] = (double)xy[1] * (aspy / aspx);
        } else {
            xy[0] = (double)xy[0] * (aspx / aspy);
        }
    }

    d_obj->x_off = xy[0] / 2;
    d_obj->y_off = xy[1] / 2;

    for (int i = 0; i < d_obj->frames; i++) {
        struct Image* image = image_create(RGBA, 0, xy[0], xy[1]);
        if (image == NULL) {
            return ERROR;
        }

        scale_rgba(d_obj->ipp[i], image);
        image_destroy(d_obj->ipp[i]);
        d_obj->ipp[i] = image;
    }

    return E_OK;
}

// Create GFX object and frames.
static int
cmd_end(struct DefObject* d_obj)
{
    int              alpha = 0, gfx = 0, cm = 0;
    struct Clip      clip;

    printf("end...\n");

    char* p;
    do {
        p = next_cmd();
        if (p == NULL) {
            return ERROR;
        }
        if (!strcmp(p, "alpha")) {
            alpha = GFX_TAG_ALPHA;
        }
        if (!strcmp(p, "gfx")) {
            gfx = GFX_TAG_GRAPHICS;
        }
        if (!strcmp(p, "cm")) {
            cm = GFX_TAG_COLLISION;
        }
    } while (strcmp(p, "!"));

    for (int i = 0; i < d_obj->frames; i++) {
        clip_auto_alpha(d_obj->ipp[i], &clip);

        struct Image* image = crop(d_obj->ipp[i], &clip);
        if (image == NULL) {
            return ERROR;
        }

        image_destroy(d_obj->ipp[i]);
        d_obj->ipp[i] = image;

        struct GfxFrame* f =
            gfx_frame_create(alpha | gfx | cm,
                             d_obj->ipp[i]->width, d_obj->ipp[i]->height,
                             d_obj->x_off - clip.x0, d_obj->y_off - clip.y0);
        if (f == NULL) {
            return ERROR;
        }

        if (cm) {
            if (gfx_create_collision(f, d_obj->ipp[i]))
                return ERROR;
        }

        if (alpha) {
            if (gfx_create_alpha(f, d_obj->ipp[i]))
                return ERROR;
        }

        if (gfx) {
            if (!alpha) {
                struct Image* image = color_rgba_to_rgb(d_obj->ipp[i]);
                if (image == NULL) {
                    return ERROR;
                }

                image_destroy(d_obj->ipp[i]);
                d_obj->ipp[i] = image;
            }
            if (gfx_create_graphics(f, d_obj->ipp[i])) {
                return ERROR;
            }
        }

        if (gfx_add_frame_to_object(f, d_obj->o)) {
            return ERROR;
        }
    }

    if (d_obj->ipp) {
        free(d_obj->ipp);
    }
    image_destroy_all();

    return E_OK;
}

int
def_run(const char* filename)
{
    struct DefObject d_obj = {0};
    struct DefFunc funcs[] = {{"object", cmd_object},
                              {"frame", cmd_frame},
                              {"rgb_to_alpha", cmd_rgb_to_alpha},
                              {"autocrop_alpha", cmd_autocrop_alpha},
                              {"scale_maxspect", cmd_scale_maxspect},
                              {"end", cmd_end},
                              {0, 0}};

    int r = read_file(filename);
    if (r) {
        return r;
    }

    char* cmd;
    while ((cmd = next_cmd())) {
        int i = 0;
        while (funcs[i].magic) {
            if (!strcmp(cmd, funcs[i].magic)) {
                r = funcs[i].func(&d_obj);
                break;
            }
            i++;
        }
        if (r) {
            break;
        }
    }

    free(d_file.mem);
    return r;
}
