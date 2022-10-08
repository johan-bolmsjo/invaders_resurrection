/* Before NEO a simple framebuffer hack was used.
 * This is the hack made to use NEO instead.
 * .. And now it's a hack for SDL as well.
 */

#include <inttypes.h>
#include "error.h"
#ifdef HAVE_LIBNEO
#include <neo/neo.h>
#else
#include <SDL.h>
#endif
#include "dg.h"

#ifdef HAVE_LIBSDL
extern int sdl_true_doublebuf;
extern SDL_Surface *sdl_screen;
extern SDL_Surface *sdl_vscreen1;
extern SDL_Surface *sdl_vscreen2;
#endif


void
dg_flip (DG *dg)
{
  int t;
#ifdef HAVE_LIBSDL
  SDL_Surface *sdl_visable;
#endif  
  
  t = dg->hid;
  dg->hid = dg->vis;
  dg->vis = t;
  
#ifdef HAVE_LIBNEO
  neo_cons_set_pan (0, DG_YRES * t);
  neo_cons_sync ();
#else
  if (sdl_true_doublebuf)
    {
      if (SDL_Flip (sdl_screen))
	panic ("Failed to flip screens.");
    }
  else
    {
      if (dg->vis)
	sdl_visable = sdl_vscreen2;
      else
	sdl_visable = sdl_vscreen1;
      
      if (SDL_BlitSurface (sdl_visable, 0, sdl_screen, 0))
	panic ("Failed to blit surface to screen.");
      
      SDL_UpdateRect (sdl_screen, 0, 0, 0, 0);
    }
#endif
}
