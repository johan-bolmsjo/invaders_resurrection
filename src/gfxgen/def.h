#pragma once

#include "clip.h"
#include "image.h"
#include "libgfx/libgfx.h"

#define DEF_MAX_CMD_SIZE 100

struct DefFile {
    char* mem;
    int size;
    int pos;
};

struct DefObject {
    struct GfxObject* o;        // Object to attach frames to
    int               frames;
    struct Image**    ipp;
    struct Clip       clip;
    int               x_off;    // Hot spot
    int               y_off;
};

struct DefFunc {
    char* magic;
    int (*func)(struct DefObject*);
};

// Execute commands from graphics definition file.
int def_run(const char* filename);
