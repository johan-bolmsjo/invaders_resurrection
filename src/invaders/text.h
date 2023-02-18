#pragma once

#include <inttypes.h>

#include "dg.h"

typedef struct _Text {
    const char* str;
    char chr;
    int colour;
    int x_off;
    int x;
    int y;
    int offset;
} Text;

int  text_decode_font();
void text_print_char_adr(char chr, uint16_t colour, uint16_t* dst);
void text_print_str_adr(const char* str, uint16_t colour, uint16_t* dst);
void text_print_str_fancy_init(Text* t, const char* str, int x_off, int x, int y);
void text_print_str_fancy(DG* di, Text* t);
