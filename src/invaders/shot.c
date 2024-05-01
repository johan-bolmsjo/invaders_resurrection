#include "shot.h"

#include <stdlib.h>
#include <inttypes.h>

#include "error.h"
#include "gids.h"
#include "shields.h"
#include "libmedia/libmedia.h"

int g_shot_obj = 0;

struct Shot {
    int               x;
    int               y;
    int               x_vector;
    int               y_vector;
    struct rgb565     color;
    uint8_t           pend_rm;  // Pending removal if set
    struct Collision* c;        // Collision callback function
    void (*cb)(void);           // Extra callback function for player shots
    struct Shot*      prev;
    struct Shot*      next;
};

static struct {
    struct MLRectDim screen_dim;
    struct Shot*     shot_list;
} shot_module;
#define M shot_module

void
shot_module_init(struct MLRectDim screen_dim)
{
    M.screen_dim = screen_dim;
}

static void
shot_destroy(struct Shot* s)
{
    if (s->cb) {
        s->cb();
        s->cb = 0;
    }

    if (s->pend_rm == 2) {
        if (s->prev) {
            s->prev->next = s->next;
        } else {
            M.shot_list = s->next;
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
collision_cb(struct Collision* a, struct Collision* b)
{
    if (b->gid == GID_SHIELD) {
        if (!shields_hit(a->x0, a->y0, -1, b->id)) {
            return 0;
        }
    }

    shot_destroy(a->id_p);
    return 1;
}

struct Shot*
shot_create(int x, int y, int x_vector, int y_vector, struct rgb565 color,
            int fatal, int gid, void (*cb)(void))
{
    if (x < 0 || x > (M.screen_dim.w - 2) ||
        y < 0 || y > (M.screen_dim.h - 2)) {
        return NULL;
    }

    struct Shot* s = malloc(sizeof(struct Shot));

    struct Collision* c = NULL;
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
    s->color = color;
    s->pend_rm = 0;
    s->c = c;
    s->cb = cb;

    if (M.shot_list) {
        s->next = M.shot_list;
        s->next->prev = s;
    } else {
        s->next = 0;
    }

    s->prev = 0;
    M.shot_list = s;
    g_shot_obj++;

    return s;
}

void
shot_draw(const struct MLGraphicsBuffer* dst)
{
    struct Shot* s = M.shot_list;
    while (s) {
        if (!s->pend_rm) {
            struct rgb565* p = ml_graphics_buffer_xy(dst, s->x, s->y);
            p[0] = s->color;
            p[1] = s->color;
            p[dst->dim.w] = s->color;
            p[dst->dim.w + 1] = s->color;
        }
        s = s->next;
    }
}

void
shot_update(void)
{
    struct Shot* s = M.shot_list;
    while (s) {
        if (s->pend_rm) {
            struct Shot* st = s;
            s = s->next;
            shot_destroy(st);
            continue;
        }

        s->x += s->x_vector;
        s->y += s->y_vector;

        if (s->x < 0 || s->x > (M.screen_dim.w - 2) ||
            s->y < 0 || s->y > (M.screen_dim.h - 2)) {
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
