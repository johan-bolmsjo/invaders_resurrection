#include "title.h"

#include "armada.h"
#include "bomber.h"
#include "libmedia/libmedia.h"
#include "libutil/array.h"
#include "runlevel.h"
#include "sprite.h"
#include "status.h"
#include "text.h"
#include "ufo.h"

struct Text {
    const char* s;
    int         x;
    int         y;
};

struct TextAnim {
    int         frames;
    bool        done;
};

static struct Ufo      ufo;
static struct Bomber   bombers[3];
static struct Sprite*  sprites[4];
static struct Text     texts[6];
static struct TextAnim texts_anim[6];
static size_t          text_index;

void
title_module_init(void)
{
    int i;
    static const char* strings[6] = {"  I N V A D E R S\n\na NoCrew production",
                                     "Mystery", " 30 pts", " 20 pts", " 10 pts",
                                     "Press fire to begin"};

    ufo_init(&ufo, 34 * 8, 22 * 8, 0);
    sprite_init(&bombers[0].sprite, gfx_object_find("bomber_3"), 0, 34 * 8, 26 * 8, 0);
    bomber_init(&bombers[0]);
    sprite_init(&bombers[1].sprite, gfx_object_find("bomber_2"), 0, 34 * 8, 30 * 8, 0);
    bomber_init(&bombers[1]);
    sprite_init(&bombers[2].sprite, gfx_object_find("bomber_1"), 0, 34 * 8, 34 * 8, 0);
    bomber_init(&bombers[2]);

    sprites[0] = &ufo.sprite;
    sprites[1] = &bombers[0].sprite;
    sprites[2] = &bombers[1].sprite;
    sprites[3] = &bombers[2].sprite;

    texts[0] = (struct Text){strings[0], 30, 15};
    for (i = 0; i < 4; i++) {
        texts[1+i] = (struct Text){strings[1+i], 41, 22 + i * 4};
    }
    texts[5] = (struct Text){strings[5], 30, 38};
}

void
title_draw(const struct MLGraphicsBuffer* dst)
{
    if (g_runlevel == RUNLEVEL_TITLE0) {
        for (size_t i = 0; i < ARRAY_SIZE(sprites); i++) {
            sprite_draw(dst, sprites[i]);
        }
        for (size_t i = 0; i <= text_index; i++) {
            const struct Text* text = &texts[i];
            texts_anim[i].done = text_print_string_animated(dst, text->s, text->x, text->y, texts_anim[i].frames);
        }
    }
}

enum GameRunState
title_update(struct MLInput* input)
{
    if (g_runlevel == RUNLEVEL_TITLE0) {
        if (input->press_quit) {
            return GameExit;
        }

        if (input->press_button_a) {
            input->press_button_a = false;
            g_next_runlevel = RUNLEVEL_TITLE1;
        } else {
            if (!texts_anim[text_index].done) {
                texts_anim[text_index].frames++;
            } else if (text_index < ARRAY_SIZE(texts)-1) {
                text_index++;
                if (text_index >= 1 && text_index <= 4) {
                    sprites[text_index - 1]->show = true;
                }
            }

            ufo_anim(&ufo);
            for (size_t i = 0; i < ARRAY_SIZE(bombers); i++) {
                bomber_anim(&bombers[i]);
            }
        }
    }

    if (g_runlevel == RUNLEVEL_TITLE1) {
        for (size_t i = 0; i < ARRAY_SIZE(sprites); i++) {
            sprites[i]->show = false;
        }
        for (size_t i = 0; i < ARRAY_SIZE(texts); i++) {
            texts_anim[i].frames = 0;
            texts_anim[i].done = false;
        }
        text_index = 0;

        armada_reset();
        status_reset();
        g_next_runlevel = RUNLEVEL_PLAY0;
    }

    return GameContinue;
}
