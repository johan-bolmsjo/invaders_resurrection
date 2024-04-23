#include "collision.h"

#include <stdlib.h>

#include "error.h"

static Collision* c_base = NULL; /* Linked list base */

int g_collision_obj = 0;

Collision*
collision_create(int id, void* id_p, int gid,
                 int (*handler)(Collision*, Collision*))
{
    Collision* c;

    c = malloc(sizeof(Collision));
    c->id = id;
    c->id_p = id_p;
    c->gid = gid;
    c->pend_rm = 0;
    c->handler = handler;

    if (c_base) {
        c->next = c_base;
        c->next->prev = c;
    } else {
        c->next = 0;
    }
    c->prev = 0;
    c_base = c;
    g_collision_obj++;

    return c;
}

void
collision_destroy(Collision* c)
{
    if (c->prev)
        c->prev->next = c->next;
    else
        c_base = c->next;
    if (c->next)
        c->next->prev = c->prev;

    g_collision_obj--;
    free(c);
}

void
collision_update_from_sprite(Collision* c, Sprite* s)
{
    struct GfxFrame* f = s->go->fpp[s->frame];

    c->x0 = s->x - f->x_off;
    c->y0 = s->y - f->y_off;
    c->x1 = c->x0 + f->width - 1;
    c->y1 = c->y0 + f->height - 1;

    c->blocks = f->c_longs;
    c->mask = f->collision;
}

// Sort all collition objects in list by x0.
// TODO(jb): The function goes out of its sorting range by one.
static void
sort_list(Collision** list, int n)
{
    int i, j;
    int v;
    Collision* t;

    if (n <= 1)
        return;

    v = list[0]->x0;
    i = 0;
    j = n;
    for (;;) {
        while (list[++i]->x0 < v && i < n)
            ;
        while (list[--j]->x0 > v)
            ;
        if (i >= j)
            break;
        t = list[i];
        list[i] = list[j];
        list[j] = t;
    }
    t = list[i - 1];
    list[i - 1] = list[0];
    list[0] = t;
    sort_list(list, i - 1);
    sort_list(list + i, n - i);
}

void
collision_detection(void)
{
    if (g_collision_obj < 2) {
        return;
    }

    // List sorted by x0
    Collision** tmp_vec = malloc(sizeof(Collision) * (g_collision_obj + 1));
    Collision *c = c_base;
    for (int i = 0; i < g_collision_obj; i++) {
        tmp_vec[i] = c;
        c = c->next;
    }

    // TODO(jb): The sort function goes out of its sorting range by one.
    Collision end_marker = {0};
    tmp_vec[g_collision_obj] = &end_marker;
    sort_list(tmp_vec, g_collision_obj);

    for (int i = 0; i < (g_collision_obj - 1); i++) {
        for (int j = i + 1; j < g_collision_obj; j++) {
            if (tmp_vec[i]->x1 < tmp_vec[j]->x0) {
                break;
            }

            if (tmp_vec[i]->y0 <= tmp_vec[j]->y1 &&
                tmp_vec[i]->y1 >= tmp_vec[j]->y0) {
                if (!tmp_vec[i]->pend_rm) {
                    tmp_vec[i]->pend_rm = tmp_vec[i]->handler(tmp_vec[i], tmp_vec[j]);
                }
                if (!tmp_vec[j]->pend_rm) {
                    tmp_vec[j]->pend_rm = tmp_vec[j]->handler(tmp_vec[j], tmp_vec[i]);
                }
            }
        }
    }

    c = c_base;
    while (c) {
        Collision* c_tmp = c;
        c = c->next;
        if (c_tmp->pend_rm) {
            collision_destroy(c_tmp);
        }
    }

    free(tmp_vec);
}
