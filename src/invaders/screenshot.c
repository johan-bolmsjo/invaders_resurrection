#include "screenshot.h"
#include "libmedia/libmedia.h"
#include "libutil/color.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <inttypes.h>

enum {
    TargaDataTypeUncompressedRGB = 2,
    TargaImageDescFlagOriginLowerLeft = 0 << 5,
    TargaImageDescFlagOriginUpperLeft = 1 << 5,
};

struct TargaHeader {
   uint8_t idlength;
   uint8_t colormaptype;
   uint8_t datatypecode;
   uint8_t colormaporigin_lo;
   uint8_t colormaporigin_hi;
   uint8_t colormaplength_lo;
   uint8_t colormaplength_hi;
   uint8_t colormapdepth;
   uint8_t x_origin_lo;
   uint8_t x_origin_hi;
   uint8_t y_origin_lo;
   uint8_t y_origin_hi;
   uint8_t width_lo;
   uint8_t width_hi;
   uint8_t height_lo;
   uint8_t height_hi;
   uint8_t bitsperpixel;
   uint8_t imagedescriptor;
};

bool
screenshot_create(const struct MLGraphicsBuffer* screen, const char* path)
{
    struct TargaHeader hdr = {
        .datatypecode = TargaDataTypeUncompressedRGB,
        .width_lo = screen->dim.w & 0xff,
        .width_hi = screen->dim.w >> 8,
        .height_lo = screen->dim.h & 0xff,
        .height_hi = screen->dim.h >> 8,
        .bitsperpixel = 24,
        .imagedescriptor = TargaImageDescFlagOriginUpperLeft,
    };

    int fd = open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1) {
        return false;
    }

    const int       num_pixels = screen->dim.w * screen->dim.h;
    const int       dst_size   = num_pixels * 3;
    uint8_t*        dst        = malloc(dst_size);
    const uint16_t* src        = ml_graphics_buffer_xy(screen, 0, 0);

    for (int i = 0; i < dst_size; i += 3) {
        struct rgb pixel = unpack_rgb565((struct rgb565){.v = *src++});
        dst[i + 0] = pixel.b << 3;
        dst[i + 1] = pixel.g << 2;
        dst[i + 2] = pixel.r << 3;
    }

    const bool ok =
        write(fd, &hdr, sizeof(hdr)) == sizeof(hdr) &&
        write(fd, dst, dst_size) == dst_size;

    close(fd);
    free(dst);
    return ok;
}
