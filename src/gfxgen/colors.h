#pragma once

#include "image.h"

void          color_bgr_to_rgb(struct Image* image);
struct Image* color_rgb_to_rgba(struct Image* image);
struct Image* color_rgba_to_rgb(struct Image* image);
void          color_make_rgb_transparent(struct Image* image, uint8_t* rgb, uint8_t* thres);
