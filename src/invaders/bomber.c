#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "all.h"


/* Initialise bomber.
 */

void
bomber_init (Bomber *b)
{
  b->count = 0;
}


/* Animate bomber.
 */

void
bomber_anim (Bomber *b)
{
  int frame = b->s.frame;
  
  b->count++;
  if (b->count > 1)
    {
      b->count = 0;
      frame++;
      if (frame >= b->s.go->frames)
	frame = 0;
    }
  
  b->s.frame = frame;
}
