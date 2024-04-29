#pragma once

#include <stdbool.h>
#include <stdint.h>

enum {
    // Game is hard-coded for this resolution.
    MLDisplayWidth  = 640,
    MLDisplayHeight = 480,
};

/// Input state tailored for the current game.
struct MLInput {
    bool press_quit;          // Window close event or similar
    bool press_button_start;  // Start game or pause
    bool press_button_back;   // Exit game screen
    bool press_button_share;  // Take screenshot
    bool press_button_fire;   // Start game or fire
    int x_axis; // -1 = left, 1 = right
};

enum MLPixelFormat {
    MLPixelFormatNone,
    MLPixelFormatRGB565,  // RGBG565 pixels in native endian
};

/// Dimensions of a rectangle (in pixels).
struct MLRectDim {
    int w;
    int h;
};

struct MLDisplayMode {
    enum MLPixelFormat format;
    struct MLRectDim   dim;
    int                refresh_rate;  //!< Refresh rate in Hz (or zero for unspecified)
};

struct MLGraphicsBuffer {
    enum MLPixelFormat format;
    struct MLRectDim   dim;
    void*              pixels;
};

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

/// Open media access library.
/// Returns true on success.
bool ml_open(void);

/// Close media access library.
void ml_close(void);

/// Get time in milliseconds since library initialization.
int64_t ml_time_milliseconds(void);

/// Pull input devices.
///
/// \note Existing button press state is not cleared.
void ml_poll_input(struct MLInput* input);

/// Open display with logical display parameters. The resulting opened
/// window may have different dimensions and color depth. Returns true
/// on success.
bool ml_open_display(struct MLDisplayMode mode);

/// Close any opened display window.
void ml_close_display(void);

/// Update display with content of draw buffer. The draw buffer is
/// expected to have the same dimensions as the opened display.
void ml_update_display(const struct MLGraphicsBuffer* draw_buf);

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

static inline int
ml_pixel_bytes(enum MLPixelFormat format) {
    switch (format) {
    case MLPixelFormatRGB565:
        return 2;
    default:
        return 0;
    }
}

struct MLGraphicsBuffer* ml_graphics_buffer_create(enum MLPixelFormat format, struct MLRectDim dim);
void                     ml_graphics_buffer_destroy(struct MLGraphicsBuffer* buf);
void                     ml_graphics_buffer_clear(const struct MLGraphicsBuffer* buf);

static inline int
ml_graphics_buffer_width_bytes(const struct MLGraphicsBuffer* buf) {
    return buf->dim.w * ml_pixel_bytes(buf->format);
}

static inline int
ml_graphics_buffer_size_bytes(const struct MLGraphicsBuffer* buf) {
    return buf->dim.w * buf->dim.h * ml_pixel_bytes(buf->format);
}

/// Get address to X,Y coordinate.
static inline void*
ml_graphics_buffer_xy(const struct MLGraphicsBuffer* buf, int x, int y) {
    return &((char*)buf->pixels)[(y * buf->dim.w + x) * ml_pixel_bytes(buf->format)];
}
