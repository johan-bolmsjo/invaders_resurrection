/* Shields.
 */

#include <libgfx.h>
#include "dg.h"
#include "prim.h"
#include "runlevel.h"
#include "sprite.h"
#include "collision.h"
#include "gids.h"

#define WIDTH   (48 + 12)  /* Use multiple of 2 */
#define HEIGHT  (32 + 12)  /* Use multiple of 2 */

static uint32_t g_shield_orig1[WIDTH / 2 * HEIGHT];
static uint32_t g_shield_orig2[WIDTH / 2 * HEIGHT];
static uint32_t g_shield_orig3[WIDTH / 2 * HEIGHT];
static uint32_t *g_shields_flash[2] = {g_shield_orig1, g_shield_orig2};

static uint32_t g_shield1[WIDTH / 2 * HEIGHT];
static uint32_t g_shield2[WIDTH / 2 * HEIGHT];
static uint32_t g_shield3[WIDTH / 2 * HEIGHT];
static uint32_t g_shield4[WIDTH / 2 * HEIGHT];
static uint32_t *g_shields[4] = {g_shield1, g_shield2, g_shield3, g_shield4};

static Collision *g_collisions[4] = {0};

static Clip g_clip1 = {128 - WIDTH / 2, DG_YRES - 80 - HEIGHT / 2,
		       WIDTH, HEIGHT, 0, 0};
static Clip g_clip2 = {256 - WIDTH / 2, DG_YRES - 80 - HEIGHT / 2,
		       WIDTH, HEIGHT, 0, 0};
static Clip g_clip3 = {384 - WIDTH / 2, DG_YRES - 80 - HEIGHT / 2,
		       WIDTH, HEIGHT, 0, 0};
static Clip g_clip4 = {512 - WIDTH / 2, DG_YRES - 80 - HEIGHT / 2,
		       WIDTH, HEIGHT, 0, 0};

static Clip *g_clips[4] = {&g_clip1, &g_clip2, &g_clip3, &g_clip4};

static uint8_t g_draw[4] = {0}, g_count[4] = {2, 2, 2, 2};


/* Collision callback routine.
 */

static int
collision_cb (Collision *a, Collision *b)
{
  if (b->gid == GID_BOMBER)
    {
      g_draw[a->id] = 0;
      g_count[a->id] = 0;
    }
  
  return 0;
}


/* Create shield data and other initialisations.
 */

void
shield_tables ()
{
  int x0, x1, i, j, v, g, b;
  uint16_t *p1, *p2, *p3;
  unsigned char shield[HEIGHT][WIDTH] = {{0}};
  
  x0 = 8 + 4;
  x1 = WIDTH - x0;
  for (i = 4; i < HEIGHT - 4; i++)
    {
      for (j = x0; j < x1; j++)
	shield[i][j] = 160;
      
      if (x0 > 4)
	{
	  x0--;
	  x1++;
	}
    }
  
  p1 = (uint16_t *)g_shield_orig1 + 1;
  p2 = (uint16_t *)g_shield_orig2 + 1;
  p3 = (uint16_t *)g_shield_orig3 + 1;
  
  for (i = 1; i < HEIGHT - 1; i++)
    {
      for (j = 1; j < WIDTH - 1; j++)
	{
	  v = shield[i - 1][j - 1] * 4;
	  v += shield[i - 1][j] * 16;
	  v += shield[i - 1][j + 1] * 4;
	  v += shield[i][j - 1] * 16;
	  v += shield[i][j] * 176;
	  v += shield[i][j + 1] * 16;
	  v += shield[i + 1][j - 1] * 4;
	  v += shield[i + 1][j] * 16;
	  v += shield[i + 1][j + 1] * 4;
	  g = v >> 12;
	  b = v >> 11;
	  if (g || b)
	    {
	      *p1++ = 65535;
	      *p2++ = (15) << 11 | ((g + 63) / 2) << 5 | (b + 31) / 2;
	    } else {
	      *p1++ = 0;
	      *p2++ = 0;
	    }
	  *p3++ = (g << 5) | b;
	}
      p1 += 2;
      p2 += 2;
      p3 += 2;
    }
}


/* Create new shields.
 */

void
shields_new ()
{
  int i, j;
  uint32_t *src, *dst;
  
  for (i = 0; i < 4; i++)
    {
      g_draw[i] = 1;
      g_count[i] = 0;  /* Used for flash animation */
      
      if (!g_collisions[i])
	{
	  g_collisions[i] = collision_create (i, 0, GID_SHIELD, collision_cb);
	  g_collisions[i]->x0 = (i + 1) * 128 - WIDTH / 2;
	  g_collisions[i]->x1 = (i + 1) * 128 + WIDTH / 2 - 1;
	  g_collisions[i]->y0 = DG_YRES - 80 - HEIGHT / 2;
	  g_collisions[i]->y1 = DG_YRES - 80 + HEIGHT / 2 - 1;
	}
      
      src = g_shield_orig3;
      dst = g_shields[i];
      
      for (j = 0; j < WIDTH / 2 * HEIGHT; j++)
	*dst++ = *src++;
    }
}


/* Delete shields.
 */

void
shields_del ()
{
  int i;
  
  for (i = 0; i < 4; i++)
    {
      g_draw[i] = 0;
      g_count[i] = 0;  /* Used to clear two times. */
    }
}


static void
make_hole (uint16_t *dst)
{
  int x, y, g, b, alpha;
  uint8_t *src;
  static uint8_t hole[9 * 9] = { 16, 8, 8, 8, 8, 8, 8, 8, 16,
				  8, 4, 0, 0, 0, 0, 0, 4,  8,
			 	  8, 0, 0, 0, 0, 0, 0, 0,  8,
				  8, 0, 0, 0, 0, 0, 0, 0,  8,
				  8, 0, 0, 0, 0, 0, 0, 0,  8,
				  8, 0, 0, 0, 0, 0, 0, 0,  8,
				  8, 0, 0, 0, 0, 0, 0, 0,  8,
				  8, 4, 0, 0, 0, 0, 0, 4,  8,
				 16, 8, 8, 8, 8, 8, 8, 8, 16 };
  
  dst -= (WIDTH * 4 + 4);
  src = hole;
  for (y = 9; y > 0; y--)
    {
      for (x = 9; x > 0; x--)
	{
	  b = *dst;
	  alpha = *src++;
	  g = ((b >> 5) * alpha) >> 4;
	  b = ((b & 31) * alpha) >> 4;
	  
	  *dst++ = (g << 5) | b;
	}
      
      dst += (WIDTH - 9);
    }
}


/* Makes hole in shield and returns 1 if it was hit otherwise 0 is
 * returned.
 *
 * y_vec = 1 means that the shot comes from above.
 * y_vec = -1 means that the shot comes from below.
 */

int
shields_hit (int x_pos, int y_pos, int y_vec, int shield)
{
  int num, stride;
  uint16_t *p = (uint16_t *)g_shields[shield];
  
  x_pos -= g_collisions[shield]->x0;
  if (x_pos < 0) x_pos = 0;
  if (x_pos >= WIDTH) x_pos = WIDTH - 1;
  y_pos -= g_collisions[shield]->y0;
  if (y_pos < 0) y_pos = 0;
  if (y_pos >= HEIGHT) y_pos = HEIGHT - 1;
  
  if (y_vec < 0)
    {
      p += ((HEIGHT - 1) * WIDTH + x_pos);
      stride = -WIDTH;
      num = HEIGHT - y_pos;
    } else {
      p += x_pos;
      stride = WIDTH;
      num = y_pos + 1;
    }
  
  for (; num > 0; num--)
    {
      if (*p)
	{
	  make_hole (p);
	  return 1;
	}
      p += stride;
    }
  
  return 0;
}


/* Copy shields to the screen.
 */

void
shields_show (DG *dg)
{
  int i;
  
  for (i = 0; i < 4; i++)
    {
      if (g_draw[i])
	{
	  if (g_count[i] < 2)
	    {
	      blit_clipped_gfx_box (dg, g_clips[i],
				    (uint16_t *)g_shields_flash[g_count[i]]);
	      g_count[i]++;
	    } else {
	      blit_clipped_gfx_box (dg, g_clips[i],
				    (uint16_t *)g_shields[i]);
	    }
	}
    }
}


/* Delete shields from the screen.
 */

void
shields_hide (DG *dg)
{
  int i;
  
  for (i = 0; i < 4; i++)
    {
      if (!g_draw[i] && g_count[i] < 2)
	{
	  if (g_collisions[i])
	    {
	      collision_destroy (g_collisions[i]);
	      g_collisions[i] = 0;
	    }
	  
	  blit_clipped_colour_box (dg, g_clips[i], 0);
	  g_count[i]++;
	}
    }
}
