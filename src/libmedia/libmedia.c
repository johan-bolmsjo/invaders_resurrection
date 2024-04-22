#include "libmedia.h"

#include <SDL.h>
#include <stddef.h>

#include "libutil/array.h"
#include "libutil/xmath.h"

static struct {
    bool init;
    struct {
        bool init;
        int  bg_index;
        SDL_Window* sdl_window;
        SDL_Surface* sdl_window_surface;
        SDL_Surface* sdl_vscreen[2];
        struct MLGraphicsBuffer vscreen[2];
        struct MLDisplayDG dg;
        uint32_t frame_start_time_ms;
    } display;
    struct {
        bool init;
    } audio;
    struct {
        bool hold_left;
        bool hold_right;
        int axis_x1;
    } input;
} ml;

bool
ml_init(void)
{
    if (!ml.init) {
        ml.init = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == 0;
        if (ml.init) {
#if 0 // TODO(jb): Joystick support
            SDL_Joystick* sdl_joystick;

            if (SDL_NumJoysticks()) {
                SDL_JoystickEventState(SDL_ENABLE);
                sdl_joystick = SDL_JoystickOpen(0);
            }
#endif

            ml.display.frame_start_time_ms = SDL_GetTicks();
        }
    }
    return ml.init;
}

void
ml_deinit(void)
{
    if (ml.init) {
        SDL_Quit();
        ml.init = false;
    }
}

static void
deinit_display()
{
    for (size_t i = 0; i < ARRAY_SIZE(ml.display.sdl_vscreen); i++) {
        if (ml.display.sdl_vscreen[i]) {
            SDL_FreeSurface(ml.display.sdl_vscreen[i]);
            ml.display.sdl_vscreen[i] = NULL;
            ml.display.vscreen[i].pixels = NULL;
            ml.display.dg.adr[i] = NULL;
        }
    }

    // The surface returned by SDL_GetWindowSurface should not be freed.
    ml.display.sdl_window_surface = NULL;

    if (ml.display.sdl_window) {
        SDL_DestroyWindow(ml.display.sdl_window);
        ml.display.sdl_window = NULL;
    }

    ml.display.bg_index = 0;
    ml.display.dg.hid = ml.display.bg_index;
    ml.display.dg.vis = ml.display.dg.hid ^ 1;
    ml.display.init = false;
}

static int
pixel_format_bpp(enum MLPixelFormat format)
{
    switch (format) {
    case MLPixelFormatRGB565:
        return 16;
    default:
        return 0;
    }
}

struct PixelFormatMasks {
    uint32_t r;
    uint32_t g;
    uint32_t b;
    uint32_t a;
};

struct PixelFormatMasks
pixel_format_masks(enum MLPixelFormat format)
{
    switch (format) {
    case MLPixelFormatRGB565:
        return (struct PixelFormatMasks){.r = 0xf800, .g = 0x07e0, .b = 0x001f, .a = 0};
    default:
        return (struct PixelFormatMasks){0};
    }
}

bool
ml_set_display_mode(const struct MLDisplayMode* requested)
{
    deinit_display();

    const int bpp = pixel_format_bpp(requested->format);
    const struct PixelFormatMasks masks = pixel_format_masks(requested->format);

    // TODO(jb): Make window SDL_WINDOW_RESIZABLE
    SDL_Window* w =  SDL_CreateWindow("Invaders Resurrection",
                                      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                      requested->width, requested->height,
                                      SDL_WINDOW_OPENGL);
    if (w == NULL) {
        goto fail;
    }

    ml.display.sdl_window = w;
    ml.display.sdl_window_surface = SDL_GetWindowSurface(ml.display.sdl_window);

    for (size_t i = 0; i < ARRAY_SIZE(ml.display.sdl_vscreen); i++) {
        SDL_Surface* vs =
            SDL_CreateRGBSurface(SDL_SWSURFACE, requested->width, requested->height,
                                 bpp, masks.r, masks.g, masks.b, masks.a);
        ml.display.sdl_vscreen[i] = vs;
        if (vs == NULL) {
            goto fail;
        }
        ml.display.vscreen[i].format = requested->format;
        ml.display.vscreen[i].width = requested->width;
        ml.display.vscreen[i].height = requested->height;
        ml.display.vscreen[i].pixels = vs->pixels;
        ml.display.dg.adr[i] = vs->pixels;
    }

    SDL_ShowCursor(SDL_DISABLE);

    ml.display.init = true;
    return true;
fail:
    deinit_display();
    return false;
}

const struct MLGraphicsBuffer*
ml_get_draw_buffer(void)
{
    if (ml.display.init) {
        return &ml.display.vscreen[ml.display.bg_index];
    }
    return NULL;
}

const struct
MLDisplayDG* ml_display_dg(void)
{
    if (ml.display.init) {
        return &ml.display.dg;
    }
    return NULL;
}

void
ml_update_screen(void)
{
    if (ml.display.init) {
        // Calculate a delay so that the game loop renders at close to MLDisplayFreq.
        // Stay shy of the target to allow syncing with the VBL, should such support
        // be available.
        const uint32_t margin_ms = 1;
        const uint32_t target_frame_time_ms = (1000 / MLDisplayFreq) - margin_ms;
        const uint32_t render_frame_time_ms = SDL_GetTicks() - ml.display.frame_start_time_ms;

        if (render_frame_time_ms < target_frame_time_ms) {
            const uint32_t delay_ms = target_frame_time_ms - render_frame_time_ms;
            SDL_Delay(delay_ms);
        }

        (void)SDL_BlitSurface(ml.display.sdl_vscreen[ml.display.bg_index], NULL, ml.display.sdl_window_surface, NULL);
        (void)SDL_UpdateWindowSurface(ml.display.sdl_window);

        ml.display.bg_index ^= 1;
        ml.display.dg.hid = ml.display.bg_index;
        ml.display.dg.vis = ml.display.dg.hid ^ 1;

        ml.display.frame_start_time_ms = SDL_GetTicks();
    }
}

bool
ml_open_audio(const struct MLAudioDeviceParams* requested)
{
    SDL_AudioSpec spec = {
        .freq = requested->freq,
        .format = AUDIO_S16SYS,
        .channels = requested->channels,
        .samples = requested->samples,
        .callback = requested->write_audio,
        .userdata = requested->userdata,
    };

    if (!ml.audio.init) {
        ml.audio.init = SDL_OpenAudio(&spec, NULL) == 0;
        return ml.audio.init;
    }
    return false;
}

void
ml_close_audio(void)
{
    if (ml.audio.init) {
        SDL_CloseAudio();
        ml.audio.init = false;
    }
}

void
ml_pause_audio(bool pause)
{
    if (ml.audio.init) {
        SDL_PauseAudio(pause ? 1 : 0);
    }
}

void
ml_lock_audio(void)
{
    if (ml.audio.init) {
        SDL_LockAudio();
    }
}

void
ml_unlock_audio(void)
{
    if (ml.audio.init) {
        SDL_UnlockAudio();
    }
}

void
ml_poll_input(struct MLInput* input)
{
    SDL_Event sdl_event;

    while (SDL_PollEvent(&sdl_event)) {
        switch (sdl_event.type) {
        case SDL_WINDOWEVENT:
            if (sdl_event.window.event == SDL_WINDOWEVENT_RESIZED) {
                // TODO(jb): Handle window resize
            }
            break;

        case SDL_KEYDOWN:
            switch (sdl_event.key.keysym.sym) {
            case SDLK_ESCAPE:
                input->press_quit = true;
                break;

            case SDLK_a:
                ml.input.hold_left = true;
                break;

            case SDLK_d:
                ml.input.hold_right = true;
                break;

            case SDLK_SPACE:
                input->press_button_a = true;
                break;

            case SDLK_p:
                // TODO(jb): Implement pause
                input->press_pause = true;
                break;

            case SDLK_f: // [[fallthrough]];
            case SDLK_F11:
                // TODO(jb): Toggle fullscreen
                break;

            case SDLK_F12:
                input->press_screenshot =  true;
                break;

            default:
                break;
            }
            break;

        case SDL_KEYUP:
            switch (sdl_event.key.keysym.sym) {
            case SDLK_a:
                ml.input.hold_left = false;
                break;

            case SDLK_d:
                ml.input.hold_right = false;
                break;

            default:
                break;
            }
            break;

        case SDL_JOYBUTTONDOWN:
            input->press_button_a = true;
            break;

        case SDL_JOYAXISMOTION:
            if (!(sdl_event.jaxis.axis & 1)) {
                ml.input.axis_x1 = clamp_int(sdl_event.jaxis.value, -32767, 32767) / 16384;
            }
            break;

        case SDL_QUIT:
            input->press_quit = true;
        }
    }

    input->axis_x1 = ml.input.axis_x1;
    if (ml.input.hold_left && !ml.input.hold_right) {
        input->axis_x1 = -1;
    }
    if (ml.input.hold_right && !ml.input.hold_left) {
        input->axis_x1 = 1;
    }
}

void
ml_graphics_buffer_clear(const struct MLGraphicsBuffer* buf)
{
    memset(buf->pixels, 0, ml_graphics_buffer_size_bytes(buf));
}
