#pragma once

#include "clip.h"
#include "image.h"
#include "libgfx/libgfx.h"

#define DEF_MAX_CMD_SIZE 100

typedef struct _DefFile {
    char* mem;
    int size;
    int pos;
} DefFile;

typedef struct _DefObject {
    GfxObject* o; /* Object to attach frames to */
    int frames;
    Image** ipp;
    Clip clip;
    int x_off; /* Hot spot */
    int y_off;
} DefObject;

typedef struct _DefFunc {
    char* magic;
    int (*func)(DefObject*);
} DefFunc;

// Execute commands from graphics definition file.
int def_run(const char* filename);
