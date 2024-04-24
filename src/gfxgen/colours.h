#pragma once

#include "image.h"

void          colour_bgr_to_rgb(struct Image* image);
struct Image* colour_rgb_to_rgba(struct Image* image);
struct Image* colour_rgba_to_rgb(struct Image* image);
void          colour_make_rgb_transparent(struct Image* image, uint8_t* rgb, uint8_t* thres);
