/* Star field.. Whohaa.
 */

#include <stdlib.h>
#include <inttypes.h>
#include "all.h"

static int x_random[X_RANDOM];
static int x_random_pos;
static int y_random[Y_RANDOM];
static int y_random_pos;
static int z_random[Z_RANDOM];
static int z_random_pos;
static Star stars[STARS];

int16_t stars_cmap[COLOURS];


static void
create_star (Star *star)
{
  int x, y, z;
  int32_t a, b;
  
  do {
    if (x_random_pos >= X_RANDOM)
      x_random_pos = 0;
    if (y_random_pos >= Y_RANDOM)
      y_random_pos = 0;
    if (z_random_pos >= Z_RANDOM)
      z_random_pos = 0;
    
    y = y_random[y_random_pos++];
    z = z_random[z_random_pos++];
    
    x = x_random[x_random_pos++];
    x = ((int32_t)256 * x) / z + (DG_XRES / 2);
  } while ((unsigned int)x >= DG_XRES);
  
  a = (int32_t)((float)(256 * 65536 * y) / z + (DG_YRES / 2));
  b = (int32_t)((float)(256 * 65536 * (y + SPEED)) / z + (DG_YRES / 2));
  
  star->colour = stars_cmap[z >> 9];
  star->y_fix = a;
  star->speed = b - a;
  star->x_off = x;
  star->adr[0] = 0;
  star->adr[1] = 0;
  star->pend_rm = 0;
}


static void
create_stars ()
{
  int i;
  
  for (i = 0; i < STARS; i++)
      create_star (&stars[i]);
}


/* Create tables and initialize star field.
 */

void
stars_tables ()
{
  int i;
  int16_t *cm;
  
  /* 320/256 * "max z". -20480 ... 20479
   */
  for (i = 0; i < X_RANDOM; i++)
    x_random[i] = (int)((40960.0 * random () / (RAND_MAX + 1.0)) - 20480);
  
  /* Above camera. -16384 ... -32767
   */
  for (i = 0; i < Y_RANDOM; i++)
    y_random[i] = (int)(16384.0 * random () / (RAND_MAX + 1.0)) - 32767;
  
  /* No divide by zero. 1 ... 16383
   */
  for (i = 0; i < Z_RANDOM; i++)
    z_random[i] = (int)(16383.0 * random () / (RAND_MAX + 1.0)) + 1;

  /* Make the first stars black to avoid the "star belt" effect.
   */
  cm = stars_cmap;
  for (i = COLOURS; i > 0; i--)
    *cm++ = 0;
  
  create_stars ();
  
  /* Just grey scales for now.
   */
  cm = stars_cmap;
  for (i = COLOURS; i > 0; i--)
    *cm++ = i << 11 | i << 6 | i;
}


/* Remove stars from display.
 */

void stars_hide (DG *dg)
{
  int i, buf_no;
  int16_t *prev_plot;
  Star *star;
  
  buf_no = dg->hid;
  
  for (i = 0; i < STARS; i++)
    {
      star = &stars[i];
      
      prev_plot = star->adr[buf_no];
      if (prev_plot)
	*prev_plot = 0;
      
      if (star->pend_rm)
	{
	  create_star (star);
	  continue;
	}
    }
}


/* Plot stars.
 */

void
stars_show (DG *dg)
{
  int buf_no, i, y;
  uint16_t *this_plot, *screen;
  Star *star;
  
  buf_no = dg->hid;
  screen = dg->adr[buf_no];
  
  for (i = 0; i < STARS; i++)
    {
      star = &stars[i];
      
      star->y_fix += star->speed;
      y = star->y_fix >> 16;
      
      if (y < 0)
	continue;
      if (y >= DG_YRES)
	{
	  star->pend_rm = 1;
	  continue;
	}
      this_plot = screen + y * DG_XRES + star->x_off;
      if (*this_plot == 0)
	{
	  star->adr[buf_no] = this_plot;
	  *this_plot = star->colour;
	} else {
	  star->adr[buf_no] = 0;
	}
    }
}
