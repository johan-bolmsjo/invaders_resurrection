/* GFX tags (don't change the values)
 */

#define GFX_TAG_OBJECT    0
#define GFX_TAG_FRAME     16
#define GFX_TAG_GRAPHICS  1
#define GFX_TAG_ALPHA     2
#define GFX_TAG_COLLISION 4

/* GFX file format. MSB byte order.
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

typedef struct _GfxFrame GfxFrame;
typedef struct _GfxObject GfxObject;

struct _GfxObject {
    uint8_t name_len;
    uint8_t* name;
    int frames;
    GfxFrame** fpp;
    GfxObject* prev;
    GfxObject* next;
};

struct _GfxFrame {
    int width;
    int height;
    int x_off;
    int y_off;
    int c_longs;
    uint16_t* graphics;
    uint8_t* alpha;
    uint32_t* collision;
};

GfxObject* gfx_get_first_object();
GfxObject* gfx_object_find(uint8_t* name);
GfxObject* gfx_object_create(uint8_t* name);
void       gfx_object_destroy(GfxObject* o);
void       gfx_object_destroy_all();
GfxFrame*  gfx_frame_create(int flags, int width, int height, int x_off, int y_off);
void       gfx_frame_destroy(GfxFrame* f);
int        gfx_add_frame_to_object(GfxFrame* f, GfxObject* o);
