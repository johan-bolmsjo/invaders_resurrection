#include "shot.h"

#include <stdlib.h>
#include <inttypes.h>

#include "error.h"
#include "gids.h"
#include "shields.h"
#include "libmedia/libmedia.h"

static Shot* shot_list = 0;

int g_shot_obj = 0;

static void
shot_destroy(Shot* s)
{
    if (s->cb) {
        s->cb();
        s->cb = 0;
    }

    if (s->pend_rm == 2) {
        if (s->prev) {
            s->prev->next = s->next;
        } else {
            shot_list = s->next;
        }

        if (s->next) {
            s->next->prev = s->prev;
        }

        free(s);
        g_shot_obj--;
    } else {
        s->pend_rm++;
    }
}

static int
collision_cb(Collision* a, Collision* b)
{
    if (b->gid == GID_SHIELD) {
        if (!shields_hit(a->x0, a->y0, -1, b->id)) {
            return 0;
        }
    }

    shot_destroy(a->id_p);
    return 1;
}

Shot*
shot_create(int x, int y, int x_vector, int y_vector, uint16_t colour,
            int fatal, int gid, void (*cb)(void))
{
    if ((unsigned int)x > (MLDisplayWidth - 2) ||
        (unsigned int)y > (MLDisplayHeight - 2)) {
        return 0;
    }

    Shot* s = malloc(sizeof(Shot));

    Collision* c = NULL;
    if (fatal) {
        c = collision_create(0, s, gid, collision_cb);
        c->x0 = x;
        c->y0 = y;
        c->x1 = x + 1;
        c->y1 = y + 1;
    }

    s->x = x;
    s->y = y;
    s->x_vector = x_vector;
    s->y_vector = y_vector;
    s->colour = colour;
    s->pend_rm = 0;
    s->adr[0] = 0;
    s->adr[1] = 0;
    s->c = c;
    s->cb = cb;

    if (shot_list) {
        s->next = shot_list;
        s->next->prev = s;
    } else {
        s->next = 0;
    }

    s->prev = 0;
    shot_list = s;
    g_shot_obj++;

    return s;
}

void
shot_draw(const DG* dg)
{
    Shot* s = shot_list;
    while (s) {
        if (!s->pend_rm) {
            uint16_t* p = dg->adr[dg->hid] + s->x + s->y * MLDisplayWidth;
            s->adr[dg->hid] = p;
            p[0] = s->colour;
            p[1] = s->colour;
            p[MLDisplayWidth] = s->colour;
            p[MLDisplayWidth + 1] = s->colour;
        }

        s = s->next;
    }
}

void
shot_update(void)
{
    Shot *s = shot_list;
    while (s) {
        if (s->pend_rm) {
            Shot* st = s;
            s = s->next;
            shot_destroy(st);
            continue;
        }

        s->x += s->x_vector;
        s->y += s->y_vector;

        if ((unsigned int)s->x > (MLDisplayWidth - 2) ||
            (unsigned int)s->y > (MLDisplayHeight - 2)) {
            shot_destroy(s);
            if (s->c)
                collision_destroy(s->c);
        } else {
            if (s->c) {
                s->c->x0 = s->x;
                s->c->y0 = s->y;
                s->c->x1 = s->x + 1;
                s->c->y1 = s->y + 1;
            }
        }

        s = s->next;
    }
}
