#pragma once

#include <stdbool.h>
#include <stdint.h>

enum {
    // Game is hard-coded for this resolution.
    MLDisplayWidth  = 640,
    MLDisplayHeight = 480,

    // Game is hard-coded for this refresh rate so it needs to be
    // simulated unless it's naturally so.
    MLDisplayFreq = 60,
};

enum MLPixelFormat {
    MLPixelFormatNone,
    MLPixelFormatRGB565,  // RGBG565 pixels in native endian
};

struct MLDisplayMode {
    enum MLPixelFormat format;
    int                width;   // Buffer width in pixels
    int                height;  // Buffer height in pixels
};

struct MLGraphicsBuffer {
    enum MLPixelFormat format;
    int                width;   // Buffer width in pixels
    int                height;  // Buffer height in pixels
    void*              pixels;
};

/// TODO(jb): Remove
/// \deprecated Compatibillity struct.
typedef struct MLDisplayDG {
    int vis;
    int hid;
    uint16_t* adr[2]; // Two screens for page flipping
} DG;

enum MLAudioFormat {
    MLAudioFormatNone,
    MLAudioFormatS16,  // Signed 16 bit sampled in native endian.
};

struct MLAudioDeviceParams {
    enum MLAudioFormat format;
    int                freq;
    int                channels; // 1: mono, 2: stereo
    int                samples;  // Audio buffer size in samples (power of 2)

    // Function invoked to refill audio buffer.
    // The supplied buffer is only valid over the duration of the call.
    // The length is in bytes and the whole buffer must be filled by the function.
    // Stereo samples are interleaved in left, right channel order.
    void (*write_audio)(void* userdata, uint8_t* buf, int len);

    // User data supplied to the write_audio callback.
    void* userdata;
};

/// Input state tailored for the current game.
struct MLInput {
    bool press_quit;
    bool press_pause;
    bool press_screenshot;
    bool press_button_a;
    int axis_x1;
    int axis_y1;
};

/// Initialize media access library.
/// Returns true on success.
bool ml_init(void);

/// Deinitialize media access library.
void ml_deinit(void);

/// Set display mode.
/// Returns true on success.
bool ml_set_display_mode(const struct MLDisplayMode* requested);

/// Get graphics buffer to draw on.
const struct MLGraphicsBuffer* ml_get_draw_buffer(void);

/// TODO(jb): Remove
/// \deprecated Compatibillity struct.
const struct MLDisplayDG* ml_display_dg(void);

/// Update screen (or window) with content of draw buffer.
void ml_update_screen(void);

/// Opens the default audio device with the requested parameters.
/// Returns true on success.
bool ml_open_audio(const struct MLAudioDeviceParams* requested);

/// Shuts down audio processing and closes the audio device.
void ml_close_audio(void);

/// Pause or resume audio playback.
/// Audio must be resumed after audio device initialization.
void ml_pause_audio(bool pause);

/// Prevent audio stream writes from happening.
/// Used to synchronize with audio updates by the game loop.
void ml_lock_audio(void);

/// Allow audio stream writes again.
void ml_unlock_audio(void);

/// Pull input devices.
///
/// \note Existing button press state is not cleared.
void ml_poll_input(struct MLInput* input);

static inline int
ml_pixel_bytes(enum MLPixelFormat format) {
    switch (format) {
    case MLPixelFormatRGB565:
        return 2;
    default:
        return 0;
    }
}

/// Clear graphics buffer.
void ml_graphics_buffer_clear(const struct MLGraphicsBuffer* buf);

/// Get width of graphics buffer in bytes.
static inline int
ml_graphics_buffer_width_bytes(const struct MLGraphicsBuffer* buf) {
    return buf->width * ml_pixel_bytes(buf->format);
}

/// Get size of graphics buffer in bytes.
static inline int
ml_graphics_buffer_size_bytes(const struct MLGraphicsBuffer* buf) {
    return buf->width * buf->height * ml_pixel_bytes(buf->format);
}

/// Get address to X,Y coordinate.
static inline void*
ml_graphics_buffer_xy(const struct MLGraphicsBuffer* buf, int x, int y) {
    return &((char*)buf->pixels)[(y * buf->width + x) * ml_pixel_bytes(buf->format)];
}
