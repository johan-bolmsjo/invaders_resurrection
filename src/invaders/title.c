#include "title.h"

#include "armada.h"
#include "bomber.h"
#include "runlevel.h"
#include "sprite.h"
#include "status.h"
#include "text.h"
#include "ufo.h"

static Ufo     ufo;
static Bomber  bombers[3];
static Sprite* sprites[4];
static Text    text;
static Text    texts[6];

void
title_module_init(void)
{
    int i;
    static const char* strings[6] = {"  I N V A D E R S\n\na NoCrew production",
                                     "Mystery", " 30 pts", " 20 pts", " 10 pts",
                                     "Press fire to begin"};

    ufo_init(&ufo, 34 * 8, 22 * 8, 0);
    sprite_init(&bombers[0].s, gfx_object_find("bomber_3"), 0, 34 * 8, 26 * 8, 0);
    bomber_init(&bombers[0]);
    sprite_init(&bombers[1].s, gfx_object_find("bomber_2"), 0, 34 * 8, 30 * 8, 0);
    bomber_init(&bombers[1]);
    sprite_init(&bombers[2].s, gfx_object_find("bomber_1"), 0, 34 * 8, 34 * 8, 0);
    bomber_init(&bombers[2]);

    sprites[0] = &ufo.s;
    sprites[1] = &bombers[0].s;
    sprites[2] = &bombers[1].s;
    sprites[3] = &bombers[2].s;

    text_print_str_fancy_init(&texts[0], strings[0], 30, 30, 15);
    for (i = 0; i < 4; i++)
        text_print_str_fancy_init(&texts[i + 1], strings[i + 1], 41, 41, 22 + i * 4);
    text_print_str_fancy_init(&texts[5], strings[5], 30, 30, 38);

    text.str = 0;
}

void
title_show(const DG* dg)
{
    int i;

    if (g_runlevel == RUNLEVEL_TITLE0) {

        for (i = 0; i < 4; i++)
            sprite_show(dg, sprites[i]);

        text_print_str_fancy(dg, &text);
    }
}

void
title_hide(const DG* dg)
{
    int i;
    Clip clip = {240, 120, 160, 192, 0, 0};

    if (g_runlevel == RUNLEVEL_TITLE0) {
        for (i = 0; i < 4; i++)
            sprite_hide(dg, sprites[i]);
    }

    if (g_runlevel == RUNLEVEL_TITLE1)
        blit_clipped_colour_box(dg, &clip, 0);
}

int
title_update(const DG* dg, struct MLInput* input)
{
    (void)dg;

    int i;
    static int count = 0;

    if (g_runlevel == RUNLEVEL_TITLE0) {
        if (input->press_quit) {
            return 1;
        }

        if (input->press_button_a) {
            input->press_button_a = false;
            count = 0;
            g_next_runlevel = RUNLEVEL_TITLE1;
        } else {

            if (!text.str && count < 6) {
                if (count >= 1 && count <= 4)
                    sprites[count - 1]->vis = 1;

                text = texts[count++];
            }

            ufo_anim(&ufo);
            for (i = 0; i < 3; i++)
                bomber_anim(&bombers[i]);
        }
    }

    if (g_runlevel == RUNLEVEL_TITLE1) {
        if (count < 2) {
            count++;
        } else {
            count = 0;
            text.str = 0;
            for (i = 0; i < 4; i++)
                sprites[i]->vis = 0;

            armada_reset();
            status_reset();
            g_next_runlevel = RUNLEVEL_PLAY0;
        }
    }

    return 0;
}
