#pragma once
/// \file text.h
///
/// Text routines.
///

#include <inttypes.h>

#include "libmedia/libmedia.h"

typedef struct _Text {
    const char* str;
    char chr;
    int colour;
    int x_off;
    int x;
    int y;
    int offset;
} Text;

/// Decode game font (8x8) from asset data.
int text_decode_font(void);

/// Print character "c" at address "dst".
void text_print_char_adr(char chr, uint16_t colour, uint16_t* dst);

/// Print string "str" at address "dst".
/// No clipping is performed.
void text_print_str_adr(const char* str, uint16_t colour, uint16_t* dst);

/// x and y is character coordinates.
/// TODO(jb): Que?¿
void text_print_str_fancy_init(Text* t, const char* str, int x_off, int x, int y);

void text_print_str_fancy(const DG* dg, Text* t);
