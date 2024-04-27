#include "libmedia.h"

#include <SDL.h>
#include <stddef.h>
#include <stdint.h>

#include "SDL_keycode.h"
#include "libutil/array.h"
#include "libutil/xmath.h"

struct SteeringInput {
    bool left;
    bool right;
};

static struct {
    bool init;
    struct {
        bool init;
        SDL_Window* sdl_window;
        SDL_Surface* sdl_window_surface;
        SDL_Surface* sdl_draw_buffer;
        struct MLGraphicsBuffer draw_buffer;
        uint32_t frame_start_time_ms;
    } display;
    struct {
        bool init;
    } audio;
    struct {
        SDL_GameController* handle;
        struct SteeringInput dpad;
        int x_axis;
    } controller;
    struct {
        struct SteeringInput wasd;
        struct SteeringInput arrows;
    } keyboard;
} ml;

static SDL_GameController*
open_first_controller() {
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            return SDL_GameControllerOpen(i);
        }
    }
    return NULL;
}

bool
ml_open(void)
{
    if (!ml.init) {
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1");

        ml.init = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) == 0;
        if (ml.init) {
            SDL_JoystickEventState(SDL_ENABLE);
            ml.controller.handle = open_first_controller();
            ml.display.frame_start_time_ms = SDL_GetTicks();
        }
    }
    return ml.init;
}

void
ml_close(void)
{
    if (ml.init) {
        if (ml.controller.handle != NULL) {
            SDL_GameControllerClose(ml.controller.handle);
            ml.controller.handle = NULL;
        }
        ml_close_audio();
        SDL_Quit();
        ml.init = false;
    }
}

static void
close_display()
{
    if (ml.display.sdl_draw_buffer) {
        SDL_FreeSurface(ml.display.sdl_draw_buffer);
        ml.display.sdl_draw_buffer = NULL;
        ml.display.draw_buffer.pixels = NULL;
    }

    // The surface returned by SDL_GetWindowSurface should not be freed.
    ml.display.sdl_window_surface = NULL;

    if (ml.display.sdl_window) {
        SDL_DestroyWindow(ml.display.sdl_window);
        ml.display.sdl_window = NULL;
    }

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
    close_display();

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

    ml.display.sdl_draw_buffer =
        SDL_CreateRGBSurface(SDL_SWSURFACE, requested->width, requested->height,
                             bpp, masks.r, masks.g, masks.b, masks.a);
    if (ml.display.sdl_draw_buffer == NULL) {
        goto fail;
    }
    ml.display.draw_buffer.format = requested->format;
    ml.display.draw_buffer.width = requested->width;
    ml.display.draw_buffer.height = requested->height;
    ml.display.draw_buffer.pixels = ml.display.sdl_draw_buffer->pixels;

    SDL_ShowCursor(SDL_DISABLE);

    ml.display.init = true;
    return true;
fail:
    close_display();
    return false;
}

const struct MLGraphicsBuffer*
ml_get_draw_buffer(void)
{
    if (ml.display.init) {
        return &ml.display.draw_buffer;
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

        (void)SDL_BlitSurface(ml.display.sdl_draw_buffer, NULL, ml.display.sdl_window_surface, NULL);
        (void)SDL_UpdateWindowSurface(ml.display.sdl_window);

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

static int
steering_input_x_axis_value(struct SteeringInput v) {
    if (v.left && !v.right) {
        return -1;
    } else if (v.right && !v.left) {
        return 1;
    }
    return 0;
}

void
ml_poll_input(struct MLInput* input)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                // TODO(jb): Handle window resize
            }
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
                input->press_button_back = true;
                break;

            case SDLK_a:
                ml.keyboard.wasd.left = true;
                break;

            case SDLK_d:
                ml.keyboard.wasd.right = true;
                break;

            case SDLK_LEFT:
                ml.keyboard.arrows.left = true;
                break;

            case SDLK_RIGHT:
                ml.keyboard.arrows.right = true;
                break;

            case SDLK_SPACE:
                input->press_button_fire = true;
                break;

            case SDLK_PAUSE:
                // Start button has start or pause semantics depending on context.
                input->press_button_start = true;
                break;

            case SDLK_F11:
                // TODO(jb): Toggle fullscreen
                break;

            case SDLK_F12:
                input->press_button_share =  true;
                break;
            }
            break;

        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_a:
                ml.keyboard.wasd.left = false;
                break;

            case SDLK_d:
                ml.keyboard.wasd.right = false;
                break;

            case SDLK_LEFT:
                ml.keyboard.arrows.left = false;
                break;

            case SDLK_RIGHT:
                ml.keyboard.arrows.right = false;
                break;
            }
            break;

        case SDL_CONTROLLERDEVICEADDED:
            if (!ml.controller.handle) {
                ml.controller.handle = SDL_GameControllerOpen(event.cdevice.which);
            }
            break;

        case SDL_CONTROLLERDEVICEREMOVED: {
            if (ml.controller.handle && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(ml.controller.handle))) {
                SDL_GameControllerClose(ml.controller.handle);
                ml.controller.handle = open_first_controller();

                // Reset controller state to neutral position.
                ml.controller.dpad.left = false;
                ml.controller.dpad.right = false;
                ml.controller.x_axis = 0;
            }
            break;
        }

        case SDL_CONTROLLERBUTTONDOWN:
            switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_START:
                input->press_button_start = true;
                break;
            case SDL_CONTROLLER_BUTTON_BACK:
                input->press_button_back = true;
                break;
            case SDL_CONTROLLER_BUTTON_MISC1:
                // TODO(jb): MISC1 is not the "share" button on all controllers. Need to check the
                //           controller type.
                input->press_button_share = true;
                break;
            case SDL_CONTROLLER_BUTTON_X:
                input->press_button_fire = true;
                break;
            }
            // fall through
        case SDL_CONTROLLERBUTTONUP:
            switch (event.cbutton.button) {
            case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                ml.controller.dpad.left = event.cbutton.state;
                break;
            case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                ml.controller.dpad.right = event.cbutton.state;
                break;
            }
            break;

        case SDL_CONTROLLERAXISMOTION:
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                ml.controller.x_axis = clamp_int(event.caxis.value, -32767, 32767) / 16384;
            }
            break;

        case SDL_QUIT:
            input->press_quit = true;
        }
    }

    // Steering prioritization of inputs:
    // Stick to one source as long as it says 'left' or 'right'.
    input->x_axis = steering_input_x_axis_value(ml.keyboard.arrows);
    if (!input->x_axis) {
        input->x_axis = steering_input_x_axis_value(ml.keyboard.wasd);
    }
    if (!input->x_axis) {
        input->x_axis = steering_input_x_axis_value(ml.controller.dpad);
    }
    if (!input->x_axis) {
        input->x_axis = ml.controller.x_axis;
    }
}

void
ml_graphics_buffer_clear(const struct MLGraphicsBuffer* buf)
{
    memset(buf->pixels, 0, ml_graphics_buffer_size_bytes(buf));
}
