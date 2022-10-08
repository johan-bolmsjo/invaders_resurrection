#pragma once

typedef struct _Clip
{
  int x0;
  int y0;
  int x1;
  int y1;
  int w;
  int h;
} Clip;

void
clip_auto_alpha (Image *image, Clip *clip);
