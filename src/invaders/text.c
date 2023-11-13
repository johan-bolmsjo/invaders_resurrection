#include "text.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <zlib.h>

#include "libutil/endian.h"

#include "font_data.c"

static uint8_t font[2048];
static uint16_t palette[3] = {65535, 40314, 17141};

int
text_decode_font(void)
{
    uLong src_len, dst_len;

    src_len = ntohl(*(uint32_t*)(font_data + 4));
    dst_len = ntohl(*(uint32_t*)font_data);

    return uncompress(font, &dst_len, font_data + 8, src_len);
}

void
text_print_char_adr(char chr, uint16_t colour, uint16_t* dst)
{
    int h, w;
    uint8_t *cp, v;

    cp = font + (int)chr * 8;

    for (h = 8; h > 0; h--) {
        v = *cp++;
        for (w = 8; w > 0; w--) {
            if (v & 0x80)
                *dst++ = colour;
            else
                *dst++ = 0;
            v <<= 1;
        }
        dst += MLDisplayWidth - 8;
    }
}

void
text_print_str_adr(const char* str, uint16_t colour, uint16_t* dst)
{
    char chr;
    while ((chr = *str++)) {
        text_print_char_adr(chr, colour, dst);
        dst += 8;
    }
}

void
text_print_str_fancy_init(Text* t, const char* str, int x_off, int x, int y)
{
    t->str = str;
    t->colour = 0;
    t->x_off = x_off;
    t->x = x;
    t->y = y;
}

void
text_print_str_fancy(const DG* dg, Text* t)
{
    if (t->str) {
        if (!t->colour) {
            t->chr = *t->str++;
            switch (t->chr) {
            case 0:
                t->str = 0;
                return;

            case 10:
                t->x = t->x_off;
                t->y++;
                t->chr = ' ';
                break;

            default:
                t->x++;
            }
            if (t->x >= (MLDisplayWidth / 8)) {
                t->x = t->x_off;
                t->y++;
            }
            if (t->y >= (MLDisplayHeight / 8)) {
                t->str = 0;
                return;
            }

            t->offset = t->x * 8 + MLDisplayWidth * t->y * 8;
        }

        if (t->colour < 3) {
            text_print_char_adr(t->chr, palette[t->colour],
                                dg->adr[dg->hid] + t->offset);
            t->colour++;
        } else {
            text_print_char_adr(t->chr, palette[2],
                                dg->adr[dg->hid] + t->offset);
            t->colour = 0;
            text_print_str_fancy(dg, t);
        }
    }
}
