#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <inttypes.h>
#include "libgfx/libgfx.h"
#include "image.h"
#include "error.h"


/* Create frame graphics from image.
 */

int
gfx_create_graphics (GfxFrame *f, Image *im)
{
  uint8_t *s;
  int16_t *d, v;
  int32_t pixels;
  
  if (im->type != RGBA && im->type != RGB)
    return ERROR;
  
  if (im->width != f->width ||
      im->height != f->height ||
      f->graphics == NULL)
    return ERROR;
  
  s = im->image;
  d = f->graphics;
  pixels = f->width * f->height;
  
  for (; pixels > 0; pixels--)
    {
      v = ((int)s[0] >> 3) << 11;
      v |= ((int)s[1] >> 2) << 5;
      v |= (int)s[2] >> 3;
      *d++ = v;
      s += im->depth;
    }
  
  return E_OK;
}


/* Create frame alpha channel mask from image.
 */

int
gfx_create_alpha (GfxFrame *f, Image *im)
{
  uint8_t *s, *d;
  int32_t pixels;
  
  if (im->type != RGBA ||
      im->width != f->width ||
      im->height != f->height ||
      f->alpha == NULL)
    return ERROR;
  
  s = im->image + 3;
  d = f->alpha;
  pixels = f->width * f->height;
  
  for (; pixels > 0; pixels--)
    {
      *d++ = *s;
      s += 4;
    }
  
  return E_OK;
}


/* Create collision mask from alpha channel in image.
 */

int
gfx_create_collision (GfxFrame *f, Image *im)
{
  int i, j, shift;
  uint8_t *s;
  int32_t block, *d;
  
  if (im->type != RGBA ||
      im->width != f->width ||
      im->height != f->height ||
      f->collision == NULL)
    return ERROR;
  
  s = im->image + 3;
  d = f->collision;
  block = 0;
  shift = 31;
  
  for (i = f->height; i > 0; i--)
    { 
      for (j = f->width - 1; j >= 0; j--)
	{
	  if (*s != 16)
	    block |= (int32_t)1 << shift;
	  s += im->depth;
	  shift--;
	  if (shift < 0 || ! j)
	    {
	      *d++ = block;
	      block = 0;
	      shift = 31;
	    }
	}
    }
  
  return E_OK;
}


/* Writes gfx data to file.
 */

int
gfx_write (GfxObject *o, char *path)
{
  int frame, w = 0, w2 = 0, i;
  uint8_t otag[2] = {GFX_TAG_OBJECT}, ftag;
  uint16_t s, sa[4], *sp;
  uint32_t l, *lp;
  gzFile gz;
  GfxFrame *fp;
  
  gz = gzopen (path, "wb9");
  if (gz == NULL)
    return E_OPEN;
  
  while (o)
    {
      otag[1] = o->name_len;
      w2 += 2 + o->name_len;
      w += gzwrite (gz, otag, 2);
      w += gzwrite (gz, o->name, o->name_len);
      
      for (frame = 0; frame < o->frames; frame++)
	{
	  fp = o->fpp[frame];
	  ftag = GFX_TAG_FRAME;
	  
	  if (fp->graphics)
	    ftag |= GFX_TAG_GRAPHICS;
	  
	  if (fp->alpha)
	    ftag |= GFX_TAG_ALPHA;
	  
	  if (fp->collision)
	    ftag |= GFX_TAG_COLLISION;
	  
	  sa[0] = msb_short (fp->width);
	  sa[1] = msb_short (fp->height);
	  sa[2] = msb_short (fp->x_off);
	  sa[3] = msb_short (fp->y_off);
	  w2 += 9;
	  w += gzwrite (gz, &ftag, 1);
	  w += gzwrite (gz, sa, 8);
	  
	  sp = fp->graphics;
	  if (sp)
	    {
	      i = fp->width * fp->height;
	      w2 += i * 2;
	      for (; i > 0; i--)
		{
		  s = msb_short (*sp++);
		  w += gzwrite (gz, &s, 2);
		}
	    }
	  
	  if (fp->alpha)
	    {
	      i = fp->width * fp->height;
	      w2 += i;
	      w += gzwrite (gz, fp->alpha, i);
	    }
	  
	  lp = fp->collision;
	  if (lp)
	    {
	      i = fp->c_longs * fp->height;
	      w2 += i * 4;
	      for (; i > 0; i--)
		{
		  l = msb_long (*lp++);
		  w += gzwrite (gz, &l, 4);
		}
	    }
	}
      
      o = o->next;
    }
  
  gzclose (gz);
  
  if (w == w2)
    return E_OK;
  else
    return E_WRITE;
}
