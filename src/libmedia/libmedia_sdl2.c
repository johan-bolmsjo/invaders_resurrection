#include "SDL_video.h"
#include "libmedia.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <stddef.h>
#include <stdint.h>

#include "SDL_keycode.h"
#include "SDL_log.h"
#include "libutil/array.h"
#include "libutil/xmath.h"

struct SteeringInput {
    bool left;
    bool right;
};

struct Coord2D {
    float x, y;
};

struct Texture {
    GLuint           id;
    struct MLRectDim dim;
    struct Coord2D   a, b;
};

static struct {
    bool init;
    struct {
        enum MLPixelFormat logical_format;
        struct MLRectDim logical_dim;
        struct MLRectDim physical_dim;
        SDL_Window* sdl_window;
        SDL_GLContext gl_context;
        struct Texture draw_texture;
        uint32_t frame_start_time_ms;
        int f_fullscreen;
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
open_first_controller(void) {
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
        ml_close_display();
        ml_close_audio();
        SDL_Quit();
        ml.init = false;
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

static void
resize_opengl(struct MLRectDim physical_dim)
{
    // Reset the current viewport and perspective transformation
    glViewport(0, 0, physical_dim.w, physical_dim.h);

    // Reset projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set parallel projection (Left, Right, Bottom, Top, Near, Far).
    // This gives a normal (0,0) at top left corner coordinate system.
    glOrtho(0, physical_dim.w, physical_dim.h, 0, -1, 1);

    glMatrixMode(GL_MODELVIEW);
}

void
ml_poll_input(struct MLInput* input)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                ml.display.physical_dim = (struct MLRectDim){.w = event.window.data1, .h = event.window.data2};
                resize_opengl(ml.display.physical_dim);
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

            case SDLK_F11: {
                ml.display.f_fullscreen ^= 1;
                const Uint32 sdl_flags = ml.display.f_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                SDL_SetWindowFullscreen(ml.display.sdl_window, sdl_flags);
                break;
            }

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

static GLenum
pixel_format_gl_mode(enum MLPixelFormat format)
{
    switch (format) {
    case MLPixelFormatRGB565:
        return GL_UNSIGNED_SHORT_5_6_5;
    default:
        return 0;
    }
}

static void
create_draw_texture(struct Texture* draw_tex, enum MLPixelFormat format, struct MLRectDim logical_dim)
{
    GLuint texture_id;
    glGenTextures(1, &texture_id);

    // OpenGL 2.x is only required to support power of two texture sizes, round to avoid troubles.
    const struct MLRectDim texture_dim = (struct MLRectDim){
        round_up_pow2_uint32(logical_dim.w), round_up_pow2_uint32(logical_dim.h)};

    *draw_tex = (struct Texture) {
        .id = texture_id,
        .dim = texture_dim,
        .a = {0, 0},
        .b = {(float)logical_dim.w / texture_dim.w, (float)logical_dim.h / texture_dim.h},
    };

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_dim.w, texture_dim.h,
                 0, GL_RGB, pixel_format_gl_mode(format), NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool
ml_open_display(enum MLPixelFormat format, struct MLRectDim dim)
{
    ml_close_display();

    ml.display.logical_format = format;
    ml.display.logical_dim = dim;
    ml.display.physical_dim = dim;

    // Use OpenGL 2.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    ml.display.sdl_window =  SDL_CreateWindow("Invaders Resurrection",
                                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                              ml.display.physical_dim.w, ml.display.physical_dim.h,
                                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (ml.display.sdl_window == NULL) {
        goto fail;
    }
    ml.display.gl_context = SDL_GL_CreateContext(ml.display.sdl_window);
    if (ml.display.gl_context == NULL) {
        goto fail;
    }

    if (SDL_GL_SetSwapInterval(1) == -1) {
        SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to set OpenGL swap interval to VSync: %s\n", SDL_GetError());
    }

    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glShadeModel(GL_SMOOTH);

    // Calculate appropriate window size based on the resolution of the
    // display the window is currently placed on.
    const int sdl_display_idx = SDL_GetWindowDisplayIndex(ml.display.sdl_window);
    SDL_DisplayMode sdl_display_mode;
    SDL_GetDesktopDisplayMode(sdl_display_idx, &sdl_display_mode);

    int r = max_int(1, min_int(sdl_display_mode.w / dim.w, sdl_display_mode.h / dim.h));
    ml.display.physical_dim.w = dim.w * r;
    ml.display.physical_dim.h = dim.h * r;

    SDL_SetWindowSize(ml.display.sdl_window, ml.display.physical_dim.w, ml.display.physical_dim.h);
    SDL_SetWindowPosition(ml.display.sdl_window, SDL_WINDOWPOS_CENTERED_DISPLAY(sdl_display_idx), SDL_WINDOWPOS_CENTERED_DISPLAY(sdl_display_idx));
    SDL_ShowWindow(ml.display.sdl_window);

    resize_opengl(ml.display.physical_dim);
    create_draw_texture(&ml.display.draw_texture, format, ml.display.logical_dim);

    SDL_ShowCursor(SDL_DISABLE);
    ml.display.frame_start_time_ms = SDL_GetTicks();
    return true;

fail:
    ml_close_display();
    return false;
}

void
ml_close_display(void)
{
    if (ml.display.gl_context) {
        SDL_GL_DeleteContext(ml.display.gl_context);
        ml.display.gl_context = NULL;
    }

    if (ml.display.sdl_window) {
        SDL_DestroyWindow(ml.display.sdl_window);
        ml.display.sdl_window = NULL;
    }
}

static void
update_draw_texture(const struct Texture* draw_tex, const struct MLGraphicsBuffer* draw_buf)
{
    glBindTexture(GL_TEXTURE_2D, draw_tex->id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, draw_buf->dim.w, draw_buf->dim.h,
                    GL_RGB, pixel_format_gl_mode(draw_buf->format), draw_buf->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void
render_draw_texture(const struct Texture* draw_tex, struct MLRectDim logical_dim, struct MLRectDim physical_dim)
{
    // Fit logical display into physical display which may have a different aspect ratio.
    double r = min_double((double)physical_dim.w / logical_dim.w,
                          (double)physical_dim.h / logical_dim.h);
    int w = (int)(r * logical_dim.w);
    int h = (int)(r * logical_dim.h);
    int xoff = (physical_dim.w - w) / 2;
    int yoff = (physical_dim.h - h) / 2;

    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, ml.display.draw_texture.id);

    glBegin(GL_QUADS);
    glTexCoord2f(draw_tex->a.x, draw_tex->a.y);
    glVertex2f(xoff, yoff);
    glTexCoord2f(draw_tex->b.x, draw_tex->a.y);
    glVertex2f(xoff + w, yoff);
    glTexCoord2f(draw_tex->b.x, draw_tex->b.y);
    glVertex2f(xoff + w, yoff + h);
    glTexCoord2f(draw_tex->a.x, draw_tex->b.y);
    glVertex2f(xoff, yoff + h);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void
ml_update_display(const struct MLGraphicsBuffer* draw_buf)
{
    if (ml.display.sdl_window) {
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

        update_draw_texture(&ml.display.draw_texture, draw_buf);
        render_draw_texture(&ml.display.draw_texture, ml.display.logical_dim, ml.display.physical_dim);
        SDL_GL_SwapWindow(ml.display.sdl_window);
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
