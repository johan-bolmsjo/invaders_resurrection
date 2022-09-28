/* Displayed status
 */

#include <string.h>
#include <inttypes.h>
#include "all.h"

#define EXTRA_LIFE_SCORE  1500

static uint16_t palette[3] = {17141, 40314, 65535};

static uint8_t pilots_str;
static uint8_t pilots_atr;
static uint8_t score_str[6];
static uint8_t score_atr[6];
static uint8_t hi_score_str[6];
static uint8_t hi_score_atr[6];

static int prev_score;
static int extra_life_counter;

unsigned int g_pilots;
unsigned int g_score;
unsigned int g_hi_score = 0;


/* Reset status.
 */

void
status_reset ()
{
  prev_score = 0;
  extra_life_counter = 0;
  g_pilots = 3;
  g_score = 0;
  pilots_str = g_pilots + '0';
  pilots_atr = 0;
  memset (score_str, '0', 6);
  memset (score_atr, 0, 6);
}


/* Clear status list.
 */

void
status_hide (DG *dg)
{
  Clip c = {0, 0, 42 * 8, 8};
  
  if (g_runlevel < RUNLEVEL_PLAY0)
    blit_clipped_colour_box (dg, &c, 0);
}


/* Update and draw status list.
 */

void
status_show (DG *dg)
{
  int i, j, k, c;
  uint16_t *dst = dg->adr[dg->hid];
  
  if (g_runlevel >= RUNLEVEL_PLAY0 &&
      g_runlevel <= RUNLEVEL_PLAY2)
    {
      extra_life_counter += (g_score - prev_score);
      prev_score = g_score;
      
      if (extra_life_counter >= EXTRA_LIFE_SCORE)
	{
	  g_pilots++;
	  extra_life_counter -= EXTRA_LIFE_SCORE;
	  
	  sfx_extra_life ();
	}
      
      if (g_pilots > 9)
	g_pilots = 9;
      
      if (g_score > 999999)
	{
	  g_score = 0;
	  extra_life_counter = 0;
	}
      
      if (g_hi_score > 999999)
	g_hi_score = 0;
      
      c = g_pilots + '0';
      if (pilots_str != c)
	{
	  pilots_str = c;
	  pilots_atr = 2;
	}
      
      j = 100000;
      k = g_score;
      for (i = 0; i < 6; i++)
	{
	  c = k / j;
	  k -= (c * j);
	  j /= 10;
	  c += '0';
	  if (c != score_str[i])
	    {
	      score_str[i] = c;
	      score_atr[i] = 2;
	    }
	}
      
      if (g_score >= g_hi_score)
	{
	  for (i = 0; i < 6; i++)
	    {
	      hi_score_str[i] = score_str[i];
	      hi_score_atr[i] = score_atr[i];
	    }
	  g_hi_score = g_score;
	}
      
      text_print_str_adr ("Pilots:", palette[0], dst);
      dst += (8 * 8);
      
      text_print_char_adr (pilots_str, palette[pilots_atr], dst);
      if (pilots_atr > 0)
	pilots_atr--;
      dst += (3 * 8);
      
      text_print_str_adr ("Score:", palette[0], dst);
      dst += (7 * 8);
      
      for (i = 0; i < 6; i++)
	{
	  text_print_char_adr (score_str[i], palette[score_atr[i]], dst);
	  if (score_atr[i] > 0)
	    score_atr[i]--;
	  dst += (1 * 8);
	}
      dst += (2 * 8);
      
      text_print_str_adr ("Hi Score:", palette[0], dst);
      dst += (10 * 8);
      
      for (i = 0; i < 6; i++)
	{
	  text_print_char_adr (hi_score_str[i], palette[hi_score_atr[i]], dst);
	  if (hi_score_atr[i] > 0)
	    hi_score_atr[i]--;
	  dst += (1 * 8);
	}
    }
}
