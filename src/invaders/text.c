/* Text routines
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <zlib.h>
#include "all.h"
#include "font_data.c"

static uint8_t font[2048];
static uint16_t palette[3] = {65535, 40314, 17141};


/* Load 8x8 font.
 */

int
text_decode_font ()
{
  uLong src_len, dst_len;
  
  src_len = ntohl (*(uint32_t *)(font_data + 4));
  dst_len = ntohl (*(uint32_t *)font_data);
  
  return uncompress (font, &dst_len, font_data + 8, src_len);
}


/* Print character "c" at address "dst".
 */

void
text_print_char_adr (uint8_t c, uint16_t colour, uint16_t *dst)
{
  int h, w;
  uint8_t *cp, v;
  
  cp = font + (int)c * 8;
  
  for (h = 8; h > 0; h--)
    {
      v = *cp++;
      for (w = 8; w > 0; w--)
	{
	  if (v & 0x80)
	    *dst++ = colour;
	  else
	    *dst++ = 0;
	  v <<= 1;
	}
      dst += DG_XRES - 8;
    }
}


/* Print string "s" at address "dst".
 * No clipping.
 */

void
text_print_str_adr (uint8_t *s, uint16_t colour, uint16_t *dst)
{
  uint8_t c;
  
  while ((c = *s++))
    {
      text_print_char_adr (c, colour, dst);
      dst += 8;
    }
}



/* x and y is character coordinates.
 */

void
text_print_str_fancy_init (Text *t, uint8_t *s, int x_off, int x, int y)
{
  t->s = s;
  t->colour = 0;
  t->x_off = x_off;
  t->x = x;
  t->y = y;
}


void
text_print_str_fancy (DG *dg, Text *t)
{
  if (t->s)
    {
      if (!t->colour)
	{
	  t->c = *t->s++;
	  switch (t->c)
	    {
	    case 0:
	      t->s = 0;
	      return;
	      
	    case 10:
	      t->x = t->x_off;
	      t->y++;
	      t->c = ' ';
	      break;
	      
	    default:
	      t->x++;
	    }
	  if (t->x >= (DG_XRES / 8))
	    {
	      t->x = t->x_off;
	      t->y++;
	    }
	  if (t->y >= (DG_YRES / 8))
	    {
	      t->s = 0;
	      return;
	    }
	  
	  t->offset = t->x * 8 + DG_XRES * t->y * 8;
	}
      
      if (t->colour < 3)
	{
	  text_print_char_adr (t->c, palette[t->colour],
			      dg->adr[dg->hid] + t->offset);
	  t->colour++;
	} else {
	  text_print_char_adr (t->c, palette[2],
			      dg->adr[dg->hid] + t->offset);
	  t->colour = 0;
	  text_print_str_fancy (dg, t);
	}
    }
}
