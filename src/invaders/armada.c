/* Invaders armada.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "all.h"


#define X_MIN    18
#define X_MAX    (DG_XRES - 18)
#define Y_MIN    (Y_MAX - 11 * HEIGHT)
#define Y_MAX    (DG_YRES - 15)

#define X_START  122  /* Start position X-axis */

#define X_STEP     4  /* Movement steps */
#define Y_STEP    30

#define WIDTH     36  /* Alien size */
#define HEIGHT    30

Armada armada;        /* Used in missiles.c as well */
static GfxObject *gfx_obj[3];

/* Vector tables for explosions.
 */

#define X_VECTORS 15
#define Y_VECTORS 16

static int x_vector[X_VECTORS];
static int y_vector[Y_VECTORS];
static int x_vector_c = 0;
static int y_vector_c = 0;


/* Collision handler.
 * Increases score, speed and plays explosion sound.
 */

static int
collision_cb (Collision *a, Collision *b)
{
  int i, x, y, xv, yv;
  Bomber *bomber;
  
  if (b->gid != GID_PLAYER_SHOT)
    return 0;
  
  bomber = (Bomber *)a->id_p;
  x = bomber->s.x;
  y = bomber->s.y;
  
  for (i = 0; i < 4; i++)
    {
      xv = x_vector[x_vector_c++];
      if (x_vector_c == X_VECTORS)
	x_vector_c = 0;
      
      do {
	yv = y_vector[y_vector_c++];
	if (y_vector_c == Y_VECTORS)
	  y_vector_c = 0;
      } while ((abs (xv) + abs (yv)) < 3);
      
      shot_create (x + xv, y + yv, xv, yv, 31 << 5, 0, 0, 0);
    }
  
  bomber->c = 0;
  x = bomber->x;
  y = bomber->y;
  
  if (y < ARMADA_Y)
    g_score += 30;
  if (y >= ARMADA_Y && y < (ARMADA_Y * 3))
    g_score += 20;
  if (y >= (ARMADA_Y * 3))
    g_score += 10;
  
  armada.alive--;
  
  if (armada.alive)
    {
      armada.alive_x[x]--;
      armada.alive_y[y]--;
      
      if (!armada.alive_x[x])
	{
	  for (; armada.lm <= armada.rm; armada.lm++)
	    if (armada.alive_x[armada.lm]) break;
	    
	  for (; armada.rm >= armada.lm; armada.rm--)
	    if (armada.alive_x[armada.rm]) break;
	}
      
      if (!armada.alive_y[y])
	{
	  for (; armada.tm <= armada.bm; armada.tm++)
	    if (armada.alive_y[armada.tm]) break;
	    
	  for (; armada.bm >= armada.tm; armada.bm--)
	    if (armada.alive_y[armada.bm]) break;
	  
	  armada.rows--;
	}
    }
  
  sfx_bomber_explode ();
  
  return 1;
}


/* Animate armada and update collision objects.
 */

static void
animate ()
{
  int i;
  Bomber *b = armada.b[0];
  
  for (i = 0; i < ARMADA_XY; i++)
    {
      if (b[i].c)
	{
	  bomber_anim (&b[i]);
	  collision_update_from_sprite (b[i].c, &b[i].s);
	}
    }
}


/* Move armada to the start position.
 */

static void
move_armada_to_start ()
{
  int i, j, x, y;
  
  y = Y_MIN + armada.y_off * HEIGHT;
  
  for (i = 0; i < ARMADA_Y; i++)
    {
      x = X_START;
      for (j = 0; j < ARMADA_X; j++)
	{
	  armada.b[i][j].s.x = x; 
	  armada.b[i][j].s.y = y; 	  
	  x += WIDTH;
	}
      y += HEIGHT;
    }
  
  armada.row = armada.bm;
  armada.row_c = 0;
  armada.row_cw = armada.alive / armada.rows;
  armada.frac = armada.alive % armada.rows;
  armada.kill = 0;
}


/* Move armada in the usual space invaders way.
 */

static void
move_armada ()
{
  int i, x = 0, y = 0;
  static int sfx_counter = 0;
  
  if (!armada.alive)
    return;
  
  sfx_counter++;
  
  if (armada.frac)
    {
      armada.frac--;
      return;
    }
  
  armada.row_c++;
  if (armada.row_c >= armada.row_cw)
    {
      armada.row_c = 0;
      
      if (armada.row < 0)
	armada.row = armada.bm; 
      
      while (!armada.alive_y[armada.row])
	{
	  armada.row--;
	  if (armada.row < 0)
	    armada.row = armada.bm; 
	}
      
      if (armada.row == armada.tm)
	{
	  armada.row_cw = armada.alive / armada.rows;
	  armada.frac = armada.alive % armada.rows;
	}
      
      if (armada.dir_r)
	{
	  if (armada.b[armada.row][armada.rm].s.x + X_STEP <= X_MAX)
	    x = X_STEP;
	  else
	    armada.dir_d = 1;
	} else {
	  if (armada.b[armada.row][armada.lm].s.x - X_STEP >= X_MIN)
	    x = -X_STEP;
	  else
	    armada.dir_d = 1;
	}
      
      if (armada.dir_d)
	{
	  if (armada.b[armada.row][armada.lm].s.y + Y_STEP == Y_MAX)
	    armada.kill = 1;
	  
	  if (armada.row == armada.tm)
	    {
	      armada.dir_d = 0;
	      armada.dir_r ^= 1;
	      
	      if (armada.kill)
		{
		  player_kill ();
		  armada.kill = 0;
		}
	    }
	  
	  y = Y_STEP;
	}
      
      if (x || y)
	{
	  if (armada.row == armada.bm && sfx_counter >= 2)
	    {
	      sfx_bomber_move (armada.alive);
	      sfx_counter = 0;
	    }
	  
	  for (i = 0; i < ARMADA_X; i++)
	    {
	      armada.b[armada.row][i].s.x += x;
	      armada.b[armada.row][i].s.y += y;
	    }
	  
	  armada.row--;
	}
    }
}


/* One-time setup of things.
 */

void 
armada_tables ()
{
  int i, j, type;
  
  for (i = 0; i < X_VECTORS;)
    {
      j = (int)(11.0 * random () / (RAND_MAX + 1.0)) - 5;
      x_vector[i++] = j;
    }
  
  for (i = 0; i < Y_VECTORS;)
    {
      j = (int)(11.0 * random () / (RAND_MAX + 1.0)) - 5;
      y_vector[i++] = j;
    }
  
  memset (&armada, 0, sizeof(Armada));
  
  gfx_obj[0] = gfx_object_find ("bomber_3");
  gfx_obj[1] = gfx_object_find ("bomber_2");
  gfx_obj[2] = gfx_object_find ("bomber_1");
  
  type = 0;
  
  for (i = 0; i < ARMADA_Y; i++)
    {
      if (i == 1)
	type = 1;
      if (i == 3)
	type = 2;
      
      for (j = 0; j < ARMADA_X; j++)
	{
	  armada.b[i][j].x = j;
	  armada.b[i][j].y = i;
	  armada.b[i][j].s.vis = 0;
	  armada.b[i][j].s.go = gfx_obj[type];
	}
    }
}


/* Create new armada.
 *
 * XXX: Doesn't update the collision positions so be sure to run animate()
 *      before collision_detection() is run.
 */

static void
armada_new ()
{
  int i;
  Bomber *b = armada.b[0];
  
  if (armada.alive)
    return;
  
  armada.y_off++;
  if (armada.y_off > 2)
    {
      armada.y_off = 0;
      armada.missiles_max++;
    }
  
  for (i = 0; i < ARMADA_X; i++)
    armada.alive_x[i] = ARMADA_Y;
  
  for (i = 0; i < ARMADA_Y; i++)
    armada.alive_y[i] = ARMADA_X;
  
  for (i = 0; i < ARMADA_XY; i++)
    {
      b[i].count = 0;
      b[i].s.frame = 0;
      b[i].c = collision_create (0, (void *)&b[i], GID_BOMBER, collision_cb);
    }
  
  armada.lm = 0;
  armada.rm = ARMADA_X - 1;
  armada.tm = 0;
  armada.bm = ARMADA_Y - 1;
  
  armada.alive = ARMADA_XY;
  armada.rows = ARMADA_Y;
  armada.vis_c = 0;
  armada.dir_r = 1;
  armada.dir_d = 0;
  
  move_armada_to_start ();
}


/* Reset some values, called from the title screen.
 */

void
armada_reset ()
{
  armada.y_off = -1;       /* armada_new () is called after this function */
  armada.missiles_max = 3;
}


/* Remove armada from the screen.
 */

void
armada_hide (DG *dg)
{
  int i;
  Bomber *b = armada.b[0];
  
  for (i = 0; i < ARMADA_XY; i++)
    sprite_hide (dg, &b[i].s);
}


/* Draw armada on the screen.
 */

void
armada_show (DG *dg)
{
  int i;
  Bomber *b = armada.b[0];
  
  for (i = 0; i < ARMADA_XY; i++)
    {
      if (b[i].c && b[i].s.vis)
	sprite_show (dg, &b[i].s);
    }
}


/* Main bombers function.
 */

void
armada_update ()
{
  int i;
  Bomber *b;
  
  if (g_runlevel == RUNLEVEL_PLAY0)
    {
      if (!armada.alive)
	{
	  armada_new ();
	  shields_new ();
	}
      
      if (armada.vis_c < ARMADA_XY)
	armada.b[0][armada.vis_c++].s.vis = 1;
      else
	g_next_runlevel = RUNLEVEL_PLAY1;
    }
  
  if (g_runlevel == RUNLEVEL_PLAY1)
    {
      if (armada.alive)
	{
	  move_armada ();
	} else {
	  g_next_runlevel = RUNLEVEL_PLAY2;
	}
    }
  
  if (g_runlevel == RUNLEVEL_PLAY2)
    {
      if (armada.vis_c)
	{
	  b = &armada.b[0][--armada.vis_c];
	  b->s.vis = 0;
	} else {
	  if (g_pilots)
	    {
	      g_next_runlevel = RUNLEVEL_PLAY0;
	      move_armada_to_start ();
	    } else {
	      for (i = 0; i < ARMADA_XY; i++)
		{
		  b = &armada.b[0][i];
		  if (b->c)
		    {
		      collision_destroy (b->c);
		      b->c = 0;
		    }
		}
	      
	      armada.alive = 0;
	      shields_del ();
	    }
	}
    }
  
  animate ();
}
