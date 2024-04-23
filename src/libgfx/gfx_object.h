#pragma once

#include <inttypes.h>

#include "libutil/color.h"

/* GFX tags (don't change the values)
 */

#define GFX_TAG_OBJECT     0
#define GFX_TAG_FRAME     16
#define GFX_TAG_GRAPHICS   1
#define GFX_TAG_ALPHA      2
#define GFX_TAG_COLLISION  4

/* GFX file format. Big endian byte order.
Object
------
uint8_t tag
uint8_t name_len
uint8_t name[name_len]

Frame
-----
uint8_t tag
int16_t width
int16_t height
int16_t x_off
int16_t y_off
[graphics data]
[alpha data]
[collision data]
*/


struct GfxFrame {
    int width;
    int height;
    int x_off;
    int y_off;
    int c_longs;
    struct rgb565* graphics;
    uint8_t* alpha;
    uint32_t* collision;
};

struct GfxObject;

struct GfxObject {
    uint8_t name_len;
    char* name;
    int frames;
    struct GfxFrame** fpp;
    struct GfxObject* prev;
    struct GfxObject* next;
};

struct GfxObject* gfx_get_first_object(void);
struct GfxObject* gfx_object_find(const char* name);
struct GfxObject* gfx_object_create(const char* name);
void              gfx_object_destroy(struct GfxObject* o);
void              gfx_object_destroy_all(void);
struct GfxFrame*  gfx_frame_create(int flags, int width, int height, int x_off, int y_off);
void              gfx_frame_destroy(struct GfxFrame* f);
int               gfx_add_frame_to_object(struct GfxFrame* f, struct GfxObject* o);
