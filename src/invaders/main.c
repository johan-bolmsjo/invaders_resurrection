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
save_screenshot(const DG* dg)
{
    const char* filename = "invaders_screenshot.tga";
    if (screenshot_create(dg, filename)) {
        printf("Saved screenshot \"%s\"\n", filename);
    } else {
        printf("Failed to save screenshot \"%s\"\n", filename);
    }
}

int
main(void)
{
    if (!decode_gfx_data()) {
        fatalf("Failed to decode graphics file!");
    }
    check_gfx_objects_exist();

    if (!text_decode_font()) {
        fatalf("Failed to decode font file!");
    }

    struct MLInput input = {0};

    srandom(time(0));

    prim_module_init();
    stars_module_init();
    ufo_module_init();
    title_module_init();
    armada_module_init();
    missiles_module_init();
    player_module_init(&input);
    mystery_module_init();
    shield_module_init();

    synth_init();

    if (!ml_init()) {
        fatalf("Failed to initialize media library!");
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

    if (!ml_set_display_mode(&(struct MLDisplayMode){
                .format = MLPixelFormatRGB565,
                .width = MLDisplayWidth,
                .height = MLDisplayHeight,
            })) {
        ml_close_audio();
        ml_deinit();
        fatalf("Failed to set display mode!");
    }

    ml_pause_audio(false);
    const DG* dg = ml_display_dg();

    enum GameRunState run_state = GameContinue;
    while (run_state == GameContinue) {
        const struct MLGraphicsBuffer* draw_buf = ml_get_draw_buffer();

        ml_poll_input(&input);

        ml_lock_audio();

        ml_graphics_buffer_clear(draw_buf);

        player_hide(dg);
        missiles_hide(dg);
        armada_hide(dg);
        mystery_hide(dg);
        shot_hide(dg);

        title_draw(dg, draw_buf);
        status_draw(draw_buf);
        shields_draw(draw_buf);
        player_show(dg);
        missiles_show(dg);
        armada_show(dg);
        mystery_show(dg);
        shot_show(dg);

        // TODO(jb): Can be drawn first when all smart object removal code has been removed.
        stars_draw(draw_buf);

        if ((run_state = title_update(&input)) == GameExit) {
            continue;
        }

        shot_update();
        armada_update();
        missiles_update();
        mystery_update();
        player_update(&input);

        collision_detection();
        runlevel_update();

        ml_unlock_audio();

        if (input.press_screenshot) {
            save_screenshot(dg);
            input.press_screenshot = false;
        }
        ml_update_screen();
    }

    ml_close_audio();
    ml_deinit();
}
