/**
 * Inavders starts here.
 * SDL support wasn't supposed to be, thereof the mess.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include "all.h"
#include "gfx.h"
#include "config.h"
#ifdef HAVE_LIBNEO
#include <neo/neo.h>
#else
#include <SDL.h>
#endif

#ifdef HAVE_LIBSDL
int sdl_true_doublebuf;
SDL_Surface *sdl_screen;
SDL_Surface *sdl_vscreen1;
SDL_Surface *sdl_vscreen2;
#endif

#define RED_MASK    0xf800
#define GREEN_MASK  0x07e0
#define BLUE_MASK   0x001f
#define ALPHA_MASK  0

int g_cheat = 0;  /* Don't look :) */


int
main (int argc, char **argv)
{
  int key_q = 0, key_z = 0, key_x = 0, x_upd = 0, key_prev = 0;
  char **cpp, *objects[8] = {"bomber_1", "bomber_2", "bomber_3",
			     "missile", "player", "score", "ufo", 0};
#ifdef HAVE_LIBNEO
  char key, key_value;
  PanicCleanUp panic_neo = {neo_close};
  NeoConsInfo cons_info;
  NeoConsMode cons_mode = {NEO_CONS_RGB16, DG_XRES, DG_YRES, DG_XRES,
			   DG_YRES * 2, 60};
  NeoJoyEvent neo_joy_event;
#else
  PanicCleanUp panic_sdl = { SDL_Quit };
  SDL_Event sdl_event;
  SDL_Joystick *sdl_joystick;
  Uint32 sdl_flags, sdl_ticks1, sdl_ticks2;
  int sdl_ticks_diff;
#endif
  Joy joy = {0};
  DG dg;
  
  if (gfx_decode2 ())
    panic ("Failed to decode graphics file.");
  
  cpp = objects;
  while (*cpp)
    {
      if (! gfx_object_find (*cpp))
	panic ("Could not find object \"%s\" in graphics file.", *cpp);
      
      cpp++;
    }
  
  if (text_decode_font ())
    panic ("Failed to decode font file.");
  
  synth_open ();
  
#ifdef HAVE_LIBNEO
  neo_joy_open ();
  
  if (!neo_cons_open (&cons_info))
    {
      panic_register_cleanup (&panic_neo);
      
      if (!neo_cons_set_mode (&cons_mode))
	{
	  dg.vfreq = cons_mode.vfreq;
	  dg.vis = 0;
	  dg.hid = 1;
	  (char *) dg.adr[0] = cons_info.mem_start;
	  (char *) dg.adr[1] = cons_info.mem_start +
	    cons_mode.line_length * cons_mode.yres;
	} else {
	  panic ("Failed to open display.");
	}
    } else {
      panic ("Failed to open display.");
    }
#else
  if (SDL_Init (SDL_INIT_TIMER |
		SDL_INIT_AUDIO |
		SDL_INIT_VIDEO |
		SDL_INIT_JOYSTICK) < 0)
    panic ("Failed to initialise SDL.");
  
  panic_register_cleanup (&panic_sdl);
  
  sdl_flags = SDL_HWSURFACE | SDL_DOUBLEBUF;
#ifdef FULLSCREEN
  sdl_flags |= SDL_FULLSCREEN;
#endif
  
  if (! SDL_VideoModeOK (DG_XRES, DG_YRES, DG_BITS, sdl_flags))
    panic ("Failed to set %d x %d 16 bit video mode.", DG_XRES, DG_YRES);
  
  if (! (sdl_screen = SDL_SetVideoMode (DG_XRES, DG_YRES, DG_BITS, sdl_flags)))
    panic ("Failed to set %d x %d 16 bit video mode.", DG_XRES, DG_YRES);
  
  /*
  if (SDL_MUSTLOCK(sdl_screen))
    panic ("No support for screens that must be locked.");
  */
  
  /* Dummy flip for detecting true double buffer mode with wait on
   * vertical retrace.
   */
  SDL_Flip (sdl_screen);
  sdl_ticks1 = SDL_GetTicks ();
  
  dg.vfreq = 60;
  dg.vis = 0;
  dg.hid = 1;
  dg.adr[0] = sdl_screen->pixels;
  SDL_Flip (sdl_screen);
  dg.adr[1] = sdl_screen->pixels;
  
  sdl_ticks2 = SDL_GetTicks ();
  
  /* If we have different adresses and a significant delay we probably
   * have a true double buffer mode. At least the way SDL 1.1.8 seems
   * to work.
   */
  if ((sdl_ticks2 - sdl_ticks1) > 4 && (dg.adr[0] != dg.adr[1]))
    sdl_true_doublebuf = 1;
  else
    {
      /* Use fake surfaces */
      sdl_true_doublebuf = 0;
      
      sdl_flags = SDL_SWSURFACE;
#ifdef FULLSCREEN
      sdl_flags |= SDL_FULLSCREEN;
#endif
      
      if (! (sdl_screen = SDL_SetVideoMode (DG_XRES, DG_YRES,
					    DG_BITS, sdl_flags)))
	panic ("Failed to set %d x %d 16 bit video mode.", DG_XRES, DG_YRES);
      
      sdl_vscreen1 = SDL_CreateRGBSurface (SDL_SWSURFACE, DG_XRES, DG_YRES,
					   DG_BITS, RED_MASK, GREEN_MASK,
					   BLUE_MASK, ALPHA_MASK);
      
      sdl_vscreen2 = SDL_CreateRGBSurface (SDL_SWSURFACE, DG_XRES, DG_YRES,
					   DG_BITS, RED_MASK, GREEN_MASK,
					   BLUE_MASK, ALPHA_MASK);
      
      if (! sdl_vscreen1 || ! sdl_vscreen2)
	panic ("Failed to create surfaces.");
      
      dg.adr[0] = sdl_vscreen1->pixels;
      dg.adr[1] = sdl_vscreen2->pixels;
    }
  
  if (sdl_screen == 0 ||
      sdl_screen->w != DG_XRES ||
      sdl_screen->h != DG_YRES ||
      sdl_screen->pitch != DG_LLEN ||
      sdl_screen->format->Rmask != RED_MASK ||
      sdl_screen->format->Gmask != GREEN_MASK ||
      sdl_screen->format->Bmask != BLUE_MASK ||
      sdl_screen->format->Amask != ALPHA_MASK)
    panic ("Failed to set %d x %d 16 bit video mode.", DG_XRES, DG_YRES);
  
  SDL_ShowCursor (SDL_DISABLE);
  SDL_EnableKeyRepeat (0, 0);
  
  if (SDL_NumJoysticks ())
    {
      SDL_JoystickEventState (SDL_ENABLE);
      sdl_joystick = SDL_JoystickOpen (0);
    }
  
  sdl_ticks1 = 0;
  
#endif
  
  srandom (time (0));
  prim_tables ();
  stars_tables ();
  ufo_tables ();
  title_tables ();
  armada_tables ();
  missiles_tables ();
  player_tables (&joy);
  mystery_tables ();
  shield_tables ();
  
  while (1)
    {
      
#ifdef HAVE_LIBNEO
      while (neo_cons_get_key (&key, &key_value))
	{
	  switch (key)
	    {
	    case NEO_KEY_ESCAPE:
	      if (key_value == 1) key_q = 1;
	      break;
	      
	    case NEO_KEY_Z:
	      x_upd = 1;
	      if (key_value < 2)
		key_z = key_value;
	      break;
	      
	    case NEO_KEY_X:
	      x_upd = 1;
	      if (key_value < 2)
		key_x = key_value;
	      break;
	      
	    case NEO_KEY_SPACE:
	      if (key_value == 1) joy.button = 1;
	      break;
	      
	    case NEO_KEY_P:
	      if (key_value == 1)
		snap_create (&dg, "invaders_snap.tga");
	      break;
	      
	    default:
	      if (key_value == 1)
		{
		  if (key_prev == NEO_KEY_4 &&
		      key == NEO_KEY_2)
		    g_cheat ^= 1;
		  
		  key_prev = key;
		}
	    }
	}
      
      while (neo_joy_read (&neo_joy_event, 0))
	{
	  switch (neo_joy_event.type)
	    {
	    case NEO_JOY_EVENT_BUTTON:
	      if (neo_joy_event.value)
		joy.button = 1;
	      break;
	      
	    case NEO_JOY_EVENT_AXIS:
	      if (! (neo_joy_event.number & 1))
		{
		  joy.x_axis = neo_joy_event.value / 16384;
		  x_upd = 0;
		}
	    }
	}
      
#else /* HAVE_LIBSDL */
      
      while (SDL_PollEvent (&sdl_event))
	{
	  switch (sdl_event.type)
	    {
	    case SDL_KEYDOWN:
	      switch (sdl_event.key.keysym.sym)
		{
		case SDLK_ESCAPE:
		  key_q = 1;
		  break;
		  
		case SDLK_z:
		  x_upd = 1;
		  key_z = 1;
		  break;
		  
		case SDLK_x:
		  x_upd = 1;
		  key_x = 1;
		  break;
		  
		case SDLK_SPACE:
		  joy.button = 1;
		  break;
		  
		case SDLK_p:
		  snap_create (&dg, "invaders_snap.tga");
		  break;
		  
		default:
		  if (key_prev == SDLK_4 &&
		      sdl_event.key.keysym.sym == SDLK_2)
		    g_cheat ^= 1;
		  
		  key_prev = sdl_event.key.keysym.sym;
		}
	      break;
	      
	    case SDL_KEYUP:
	      switch (sdl_event.key.keysym.sym)
		{
		case SDLK_z:
		  x_upd = 1;
		  key_z = 0;
		  break;
		  
		case SDLK_x:
		  x_upd = 1;
		  key_x = 0;
		  
		default:
		  break;
		}
	      break;
	      
	    case SDL_JOYBUTTONDOWN:
	      joy.button = 1;
	      break;
	      
	    case SDL_JOYAXISMOTION:
	      if (! (sdl_event.jaxis.axis & 1))
		{
		  joy.x_axis = sdl_event.jaxis.value / 16384 ;
		  x_upd = 0;
		}
	      break;
	      
	    case SDL_QUIT:
	      key_q = 1;
	    }
	}
#endif
      
      /* Make sure ship is controlled properly in X direction when
       * using keys.
       */
      if (x_upd)
	{
	  if (key_z != key_x)
	    {
	      if (key_z)
		joy.x_axis = -1;
	      else
		joy.x_axis = 1;
	    } else {
	      joy.x_axis = 0;
	    }
	  x_upd = 0;
	}
      
      stars_hide (&dg);
      
      title_hide (&dg);
      status_hide (&dg);
      shields_hide (&dg);
      player_hide (&dg);
      missiles_hide (&dg);
      armada_hide (&dg);
      mystery_hide (&dg);
      shot_hide (&dg);
      
      title_show (&dg);
      status_show (&dg);
      shields_show (&dg);
      player_show (&dg);
      missiles_show (&dg);
      armada_show (&dg);
      mystery_show (&dg);
      shot_show (&dg);
      
      stars_show (&dg);
      
      if (title_update (&dg, &joy, key_q))
	break;
      
      shot_update ();
      armada_update ();
      missiles_update ();
      mystery_update ();
      player_update (&joy, &key_q);
      
      collision_detection ();
      synth_update ();
      runlevel_update ();
      
      dg_flip (&dg);
      
#ifdef HAVE_LIBSDL
      /* BUG: Should have thought about timing better. Lets make 80Hz the
       *      maximum frame rate.
       * Two scenarios:
       * 1) Vsync, but to high refresh rate.
       * 2) No vsync.
       */
      
      if (! sdl_true_doublebuf)
	{
	  sdl_ticks2 = SDL_GetTicks ();
	  sdl_ticks_diff = sdl_ticks1 - sdl_ticks2;
	  
	  if (sdl_ticks_diff < -100)
	    sdl_ticks1 = sdl_ticks2;
	  else if (sdl_ticks_diff > 0)
	    SDL_Delay (sdl_ticks_diff);
	  
	  sdl_ticks1 += 16;
	}
#endif
    }
  
  synth_close ();
  
#ifdef HAVE_LIBNEO
  neo_close ();
#else
  SDL_Quit ();
#endif
  
  return E_OK;
}
