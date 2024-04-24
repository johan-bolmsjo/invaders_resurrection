#include "status.h"

#include <string.h>
#include <inttypes.h>

#include "libmedia/libmedia.h"
#include "libutil/color.h"
#include "prim.h"
#include "runlevel.h"
#include "sfx.h"
#include "text.h"

#define EXTRA_LIFE_SCORE 1500

static const struct rgb565 palette[3] = {{17141}, {40314}, {65535}};

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

void
status_reset(void)
{
    prev_score = 0;
    extra_life_counter = 0;
    g_pilots = 3;
    g_score = 0;
    pilots_str = g_pilots + '0';
    pilots_atr = 0;
    memset(score_str, '0', 6);
    memset(score_atr, 0, 6);
}

void
status_draw(const struct MLGraphicsBuffer* dst)
{
    struct rgb565* dst_p = ml_graphics_buffer_xy(dst, 0, 0);

    if (g_runlevel >= RUNLEVEL_PLAY0 &&
        g_runlevel <= RUNLEVEL_PLAY2) {
        extra_life_counter += (g_score - prev_score);
        prev_score = g_score;

        if (extra_life_counter >= EXTRA_LIFE_SCORE) {
            g_pilots++;
            extra_life_counter -= EXTRA_LIFE_SCORE;

            sfx_extra_life();
        }

        if (g_pilots > 9)
            g_pilots = 9;

        if (g_score > 999999) {
            g_score = 0;
            extra_life_counter = 0;
        }

        if (g_hi_score > 999999) {
            g_hi_score = 0;
        }

        int c = g_pilots + '0';
        if (pilots_str != c) {
            pilots_str = c;
            pilots_atr = 2;
        }

        int j = 100000;
        int k = g_score;
        for (int i = 0; i < 6; i++) {
            c = k / j;
            k -= (c * j);
            j /= 10;
            c += '0';
            if (c != score_str[i]) {
                score_str[i] = c;
                score_atr[i] = 2;
            }
        }

        if (g_score >= g_hi_score) {
            for (int i = 0; i < 6; i++) {
                hi_score_str[i] = score_str[i];
                hi_score_atr[i] = score_atr[i];
            }
            g_hi_score = g_score;
        }

        text_print_string_at_address("Pilots:", palette[0], dst_p);
        dst_p += (8 * 8);

        text_print_char_at_address(pilots_str, palette[pilots_atr], dst_p);
        if (pilots_atr > 0) {
            pilots_atr--;
        }
        dst_p += (3 * 8);

        text_print_string_at_address("Score:", palette[0], dst_p);
        dst_p += (7 * 8);

        for (int i = 0; i < 6; i++) {
            text_print_char_at_address(score_str[i], palette[score_atr[i]], dst_p);
            if (score_atr[i] > 0) {
                score_atr[i]--;
            }
            dst_p += (1 * 8);
        }
        dst_p += (2 * 8);

        text_print_string_at_address("Hi Score:", palette[0], dst_p);
        dst_p += (10 * 8);

        for (int i = 0; i < 6; i++) {
            text_print_char_at_address(hi_score_str[i], palette[hi_score_atr[i]], dst_p);
            if (hi_score_atr[i] > 0) {
                hi_score_atr[i]--;
            }
            dst_p += (1 * 8);
        }
    }
}
