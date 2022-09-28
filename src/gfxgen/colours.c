#include <inttypes.h>
#include "image.h"


/* bgr to rgb, fix targa code later instead of this hack!
 */

void
colour_bgr_to_rgb (Image *image)
{
  int i;
  uint8_t *p = image->image, c;
  
  for (i = image->width * image->height; i > 0; i--)
    {
      c = p[0];
      p[0] = p[2];
      p[2] = c;
      p += 3;
    }
}


/* Add an alpha channel to a rgb image,
 * store as rgba.
 */

Image *
colour_rgb_to_rgba (Image *image)
{
  int i;
  uint8_t *s, *d;
  Image *image2;
  
  image2 = image_create (RGBA, 0, image->width, image->height);
  if (! image2)
    return image2;
  
  s = image->image;
  d = image2->image;
  
  for (i = image->width * image->height; i > 0; i--)
    {
      *d++ = *s++;
      *d++ = *s++;
      *d++ = *s++;
      *d++ = 0;
    }
  
  return image2;
}


/* Mix each pixel with its alpha channel and black,
 * store as rgb.
 */

Image *
colour_rgba_to_rgb (Image *image)
{
  int i, a;
  uint8_t *s, *d;
  Image *image2;
  
  image2 = image_create (RGB, 0, image->width, image->height);
  if (! image2)
    return image2;
  
  s = image->image;
  d = image2->image;
  
  for (i = image->width * image->height; i > 0; i--)
    {
      a = (16 - s[3]) << 4;
      *d++ = (s[0] * a) >> 8;
      *d++ = (s[1] * a) >> 8;
      *d++ = (s[2] * a) >> 8;
      s += 4;
    }
  
  return image2;
}


/* Game specific alpha.
 */

void
colour_make_rgb_transparent (Image *image, uint8_t *rgb, uint8_t *thres)
{
  int i, j, min, max;
  uint8_t *p = image->image;
  
  if (image->type != RGBA)
    return;
  
  for (i = image->width * image->height; i > 0; i--)
    {
      for (j = 0; j < 3; j++)
	{
	  min = (int)rgb[j] - thres[j];
	  if (min < 0) min = 0;
	  max = (int)rgb[j] + thres[j];
	  if (max > 255) max = 255;
	  
	  if (p[j] < min || p[j] > max)
	    break;
	}
      if (j == 3)
	p[j] = 16;
      p += 4;
    }
}
