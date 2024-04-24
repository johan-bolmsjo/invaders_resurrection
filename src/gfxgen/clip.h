#pragma once

#include "image.h"

struct Clip {
    int x0;
    int y0;
    int x1;
    int y1;
    int w;
    int h;
};

void clip_auto_alpha(struct Image* image, struct Clip* clip);
