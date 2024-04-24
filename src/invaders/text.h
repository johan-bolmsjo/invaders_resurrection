#pragma once
/// \file text.h
///
/// Text routines.
///

#include <inttypes.h>

#include "libmedia/libmedia.h"
#include "libutil/color.h"

enum {
    CharWidth  = 8,  // Character width in pixels
    CharHeight = 8   // Character height in pixels
};

/// Decode game font (CharWidth, CharHeight) from asset data.
/// Returns true on success.
bool text_decode_font(void);

/// Print character [c] at address [addr].
void text_print_char_at_address(char c, struct rgb565 color, struct rgb565* addr);

/// Print string [s] at address [addr].
/// \note No clipping is performed.
void text_print_string_at_address(const char* s, struct rgb565 color, struct rgb565* addr);

/// Print string [s] in an animated fashion at character coordinates [x]
/// and [y] in graphics buffer [dst]. The [frames] parameter determines
/// how many frames to emit before stop drawing. The caller is expected
/// to increment frames for each call to this function. Returns true
/// when the animation is complete.
bool text_print_string_animated(const struct MLGraphicsBuffer* dst, const char* s, int x, int y, int frames);
