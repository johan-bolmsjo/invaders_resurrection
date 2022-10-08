/* Scale RGBA image with game specific alpha channel.
 */

#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include "image.h"


static inline
double xround (double x)
{
  x += 0.5;
  return floor(x);
}


/* Slow slow...
 */

void
scale_rgba (Image *src, Image *dst)
{
  uint8_t *s, *d, r, g, b, a;
  int x, y, xi, xi2, yi, stride;
  double xp, yp, xpt, ypt, xpt2, ypt2, xpn, ypn, yp_max, xp_max,
    xx, yy, div_a, div_rgb, mul, ra, ga, ba, aa;
  
  if (src->type != RGBA || dst->type != RGBA)
    return;
  
  yp = 0;
  d = dst->image;
  xx = (double)src->width / dst->width;
  yy = (double)src->height / dst->height;
  div_a = xx * yy;
  
  xp_max = (double)src->width;
  yp_max = (double)src->height;
  
  for (y = 0; y < dst->height; y++)
    {
      xp = 0;
      ypn = yp + yy;
      if (ypn > yp_max)
	ypn = yp_max;
      
      for (x = 0; x < dst->width; x++)
	{
	  ypt = yp;
	  ypt2 = ceil (yp);
	  if (ypt2 == yp)
	    ypt2++;
	  
	  s = src->image + (int)yp * src->width * src->depth +
	    (int)xp * src->depth;
	  
	  xpn = xp + xx;
	  if (xpn > xp_max)
	    xpn = xp_max;
	  
	  xi = ceil (xpn) - floor (xp);
	  stride = (src->width - xi) * src->depth;
	  
	  ra = 0;
	  ga = 0;
	  ba = 0;
	  aa = 0;
	  div_rgb = 0;
	  
	  for (yi = ceil (ypn) - floor (yp); yi > 0; yi--)
	    {
	      if (ypt2 > ypn)
		ypt2 = ypn;
	      
	      xpt = xp;
	      xpt2 = ceil (xp);
	      if (xpt2 == xp)
		xpt2++;
	      
	      for (xi2 = xi; xi2 > 0; xi2--)
		{
		  if (xpt2 > xpn)
		    xpt2 = xpn;
		  
		  mul = (xpt2 - xpt) * (ypt2 - ypt);
		  r = *s++;
		  g = *s++;
		  b = *s++;
		  a = *s++;
		  if (a)
		    {
		      aa += (double)a * mul;
		    } else {
		      ra += (double)r * mul;
		      ga += (double)g * mul;
		      ba += (double)b * mul;
		      div_rgb += mul;
		    }
		  
		  xpt = xpt2;
		  xpt2++;
		}
	      
	      s += stride;
	      ypt = ypt2;
	      ypt2++;
	    }
	  
	  if (div_rgb)
	    {
	      r = xround (ra / div_rgb);
	      g = xround (ga / div_rgb);
	      b = xround (ba / div_rgb);
	    } else {
	      /* To avoid gcc warnings */
	      r = 0;
	      g = 0;
	      b = 0;
	    }

	  a = xround (aa / div_a);
	  if (a >= 16)
	    {
	      *d++ = 0;
	      *d++ = 0;
	      *d++ = 0;
	      *d++ = 16;
	    } else {
	      *d++ = r;
	      *d++ = g;
	      *d++ = b;
	      *d++ = a;
	    }
	  
	  xp = xpn;
	}
      
      yp = ypn;
    }
}
