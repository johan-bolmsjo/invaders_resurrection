#pragma once

#include "image.h"

struct FileFormat {
    const char** exts;
    struct Image* (*func)(char*);
};

struct Image* ff_read(char* path);
