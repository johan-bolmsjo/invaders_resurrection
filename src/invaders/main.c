/**
 * Inavders starts here.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "armada.h"
#include "gfx.h"
#include "libmedia/libmedia.h"
#include "libsynth/libsynth.h"
#include "libutil/prng.h"
#include "missiles.h"
#include "mystery.h"
#include "player.h"
#include "runlevel.h"
#include "screenshot.h"
#include "shields.h"
#include "shot.h"
#include "stars.h"
#include "status.h"
#include "text.h"
#include "title.h"
#include "ufo.h"

/// Print error message and exit application.
static void
fatalf(const char* format, ...)
{
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "E: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(1);
}

static void
write_audio_cb(void* userdata, uint8_t* buf, int len)
{
    (void)userdata;
    synth_mix((int16_t*)buf, len / sizeof(int16_t));
}

static void
check_gfx_objects_exist(void)
{
    const char *objects[] = {
        "bomber_1", "bomber_2", "bomber_3", "missile", "player", "score", "ufo", NULL
    };
    for (const char** pp = objects; *pp != NULL; pp++) {
        if (!gfx_object_find(*pp)) {
            fatalf("Could not find object \"%s\" in graphics data!", *pp);
        }
    }
}

static void
save_screenshot(const struct MLGraphicsBuffer* screen)
{
    const char* filename = "invaders_screenshot.tga";
    if (screenshot_create(screen, filename)) {
        printf("Saved screenshot \"%s\"\n", filename);
    } else {
        printf("Failed to save screenshot \"%s\"\n", filename);
    }
}

int
main(void)
{
    const struct MLRectDim screen_dim = (struct MLRectDim){.w = 640, .h = 480};

    if (!decode_gfx_data()) {
        fatalf("Failed to decode graphics file!");
    }
    check_gfx_objects_exist();

    struct MLInput input = {0};

    if (!text_module_init(screen_dim)) {
        fatalf("Failed to initialize text module!");
    }

    struct prng64_state prng_state;
    prng64_seed(&prng_state);

    prim_module_init(screen_dim);
    stars_module_init(screen_dim, &prng_state);
    ufo_module_init();
    title_module_init();
    armada_module_init(screen_dim, &prng_state);
    missiles_module_init(screen_dim, &prng_state);
    player_module_init(screen_dim, &input, &prng_state);
    mystery_module_init(screen_dim, &prng_state);
    shield_module_init(screen_dim);
    shot_module_init(screen_dim);

    synth_module_init(&prng_state);

    if (!ml_open()) {
        fatalf("Failed to open media library!");
    }

    if (!ml_open_audio(&(struct MLAudioDeviceParams){
                .format = MLAudioFormatS16,
                .freq = SynthFrequency,
                .channels = 1,
                .samples = 256,
                .write_audio = write_audio_cb,
            })) {
        // Perhaps we could do without audio?
        fatalf("Failed to open audio device!");
    }

    struct MLGraphicsBuffer* draw_buf = ml_graphics_buffer_create(MLPixelFormatRGB565, screen_dim);
    if (!ml_open_display((struct MLDisplayMode){.format = draw_buf->format, .dim = draw_buf->dim, .refresh_rate = 60})) {
        ml_close();
        fatalf("Failed to set display mode!");
    }

    ml_pause_audio(false);

    for (;;) {
        ml_poll_input(&input);
        if (input.press_quit) {
            break;
        }

        ml_lock_audio();

        ml_graphics_buffer_clear(draw_buf);
        stars_draw(draw_buf);
        title_draw(draw_buf);
        status_draw(draw_buf);
        shields_draw(draw_buf);
        player_draw(draw_buf);
        missiles_draw(draw_buf);
        armada_draw(draw_buf);
        mystery_draw(draw_buf);
        shot_draw(draw_buf);

        if (title_update(&input) == GameExit) {
            ml_unlock_audio();
            break;
        }

        shot_update();
        armada_update();
        missiles_update();
        mystery_update();
        player_update(&input);

        collision_detection();
        runlevel_update();

        ml_unlock_audio();

        if (input.press_button_share) {
            save_screenshot(draw_buf);
            input.press_button_share = false;
        }
        ml_update_display(draw_buf);
    }

    ml_graphics_buffer_destroy(draw_buf);
    ml_close();
}
