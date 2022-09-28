#include <stdlib.h>
#include <inttypes.h>
#include "all.h"


static Shot *base = 0;

int g_shot_obj = 0;


/* Destroy shot
 */

static void
shot_destroy (Shot *s)
{
  if (s->cb)
    {
      s->cb ();
      s->cb = 0;
    }
  
  if (s->pend_rm == 2)
    {
      if (s->prev)
	s->prev->next = s->next;
      else
	base = s->next;
      
      if (s->next)
	s->next->prev = s->prev;
      
      free (s);
      g_shot_obj--;
    }
  
  s->pend_rm++;
}


/* Collision callback function.
 */

static int
shot_callback (Collision *a, Collision *b)
{
  if (b->gid == GID_SHIELD)
    {
      if (!shields_hit (a->x0, a->y0, -1, b->id))
	return 0;
    }
  
  shot_destroy (a->id_p);
  return 1;
}


/* Create shot.
 */

Shot *
shot_create (int x, int y, int x_vector, int y_vector, int16_t colour,
	     int fatal, int gid, void (*cb)())
{
  Collision *c = 0;
  Shot *s;
  
  if ((unsigned int)x > (DG_XRES - 2) ||
      (unsigned int)y > (DG_YRES - 2))
    return 0;
  
  s = malloc (sizeof(Shot));
  if (!s)
    panic ("Out of memory.");
  
  if (fatal)
    {
      c = collision_create (0, s, gid, shot_callback);
      c->x0 = x;
      c->y0 = y;
      c->x1 = x + 1;
      c->y1 = y + 1;
    }
  
  s->x = x;
  s->y = y;
  s->x_vector = x_vector;
  s->y_vector = y_vector;
  s->colour = colour;
  s->pend_rm = 0;
  s->adr[0] = 0;
  s->adr[1] = 0;
  s->c = c;
  s->cb = cb;
  
  if (base)
    {
      s->next = base;
      s->next->prev = s;
    } else {
      s->next = 0;
    }
  
  s->prev = 0;
  base = s;
  g_shot_obj++;
  
  return s;
}


/* Hide all shots
 */

void
shot_hide (DG *dg)
{
  int16_t *p;
  Shot *s = base;
  
  while (s)
    {
      p = s->adr[dg->hid];
      if (p)
	{
	  p[0] = 0;
	  p[1] = 0;
	  p[DG_XRES] = 0;
	  p[DG_XRES + 1] = 0;
	}
      
      s = s->next;
    }
}


/* Show all shots
 */

void
shot_show (DG *dg)
{
  int16_t *p;
  Shot *s = base;
  
  while (s)
    {
      if (!s->pend_rm)
	{
	  p = dg->adr[dg->hid] + s->x + s->y * DG_XRES;
	  s->adr[dg->hid] = p;
	  p[0] = s->colour;
	  p[1] = s->colour;
	  p[DG_XRES] = s->colour;
	  p[DG_XRES + 1] = s->colour;
	}
      
      s = s->next;
    }
}


/* Update all shots.
 */

void
shot_update ()
{
  Shot *s = base, *st;
  
  while (s)
    {
      if (s->pend_rm)
	{
	  st = s;
	  s = s->next;
	  shot_destroy (st);
	  continue;
	}
      
      s->x += s->x_vector;
      s->y += s->y_vector;
      
      if ((unsigned int)s->x > (DG_XRES - 2) ||
	  (unsigned int)s->y > (DG_YRES - 2))
	{
	  shot_destroy (s);
	  if (s->c)
	    collision_destroy (s->c);
	} else {
	  if (s->c)
	    {
	      s->c->x0 = s->x;
	      s->c->y0 = s->y;
	      s->c->x1 = s->x + 1;
	      s->c->y1 = s->y + 1;
	    }
	}
      
      s = s->next;
    }
}
