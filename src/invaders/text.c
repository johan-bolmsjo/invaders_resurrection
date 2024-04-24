#include "text.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>
#include <zlib.h>

#include "libmedia/libmedia.h"
#include "libutil/array.h"
#include "libutil/endian.h"
#include "libutil/xmath.h"

#include "compressed_font_data.c"

static uint8_t font_data[2048];

bool
text_decode_font(void)
{
    uLong src_len, dst_len;

    src_len = ntohl(*(uint32_t*)(compressed_font_data + 4));
    dst_len = ntohl(*(uint32_t*)compressed_font_data);

    return uncompress(font_data, &dst_len, compressed_font_data + 8, src_len) == Z_OK;
}

void
text_print_char_at_address(char c, struct rgb565 color, struct rgb565* addr)
{
    const struct rgb565 black = pack_rgb565(rgb565_color_black());
    int h, w;
    uint8_t *cp, v;

    cp = &font_data[(int)c * 8];

    for (h = 8; h > 0; h--) {
        v = *cp++;
        for (w = 8; w > 0; w--) {
            if (v & 0x80) {
                *addr++ = color;
            } else {
                *addr++ = black;
            }
            v <<= 1;
        }
        addr += MLDisplayWidth - CharWidth;
    }
}

void
text_print_string_at_address(const char* s, struct rgb565 color, struct rgb565* addr)
{
    char c;
    while ((c = *s++)) {
        text_print_char_at_address(c, color, addr);
        addr += CharWidth;
    }
}

static inline bool
is_char_space(char c) {
    return c == '\n' || c == ' ';
}

bool
text_print_string_animated(const struct MLGraphicsBuffer* dst, const char* s, int x, int y, int frames)
{
    static const struct rgb565 palette[3] = {{65535}, {40314}, {17141}};
    const int char_frame_time = ARRAY_SIZE(palette);
    const int xbegin = x;
    const int xmax = dst->width / CharWidth - 1;
    const int ymax = dst->height / CharHeight - 1;

    bool space = false;
    char c;
    while ((c = *s++) && frames > 0) {
        switch (c) {
        case '\n':
            x = xbegin;
            y++;
            break;
        case ' ':
            x++;
            break;
        default: {
            const struct rgb565 color = palette[min_int(frames, char_frame_time) - 1];
            if (x <= xmax && y <= ymax) {
                text_print_char_at_address(c, color, ml_graphics_buffer_xy(dst, x * CharWidth, y * CharHeight));
            }
            x++;
        }
        }

        // Consecutive spaces render in constant time.
        if (!space || !is_char_space(c)) {
            frames -= char_frame_time;
        }
        space = is_char_space(c);
    }
    return !c && frames >= 0; // End of string; done
}
