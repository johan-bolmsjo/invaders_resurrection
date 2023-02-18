/* Collision detection
 */

#include "collision.h"

#include <stdlib.h>

#include "error.h"

static Collision* c_base = 0;  /* Linked list base */
static Collision** c_list = 0; /* List sorted by x0 */

int g_collision_obj = 0;

/* Creates collision object with a callback function that is called
 * uppon collision.
 */

Collision*
collision_create(int id, void* id_p, int gid,
                 int (*handler)(Collision*, Collision*))
{
    Collision* c;

    c = malloc(sizeof(Collision));
    if (!c)
        panic("Out of memory.");

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

/* Removes a collition object.
 */

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

/* Uppdate a collition object with data from a sprite.
 */

void
collision_update_from_sprite(Collision* c, Sprite* s)
{
    GfxFrame* f = s->go->fpp[s->frame];

    c->x0 = s->x - f->x_off;
    c->y0 = s->y - f->y_off;
    c->x1 = c->x0 + f->width - 1;
    c->y1 = c->y0 + f->height - 1;

    c->blocks = f->c_longs;
    c->mask = f->collision;
}

/* Sort all collition objects in list by x0.
 * XXX: The function goes out of its sorting range by one.
 */

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

/* Scans collision object list and calls call-back functions uppon
 * collision.
 */

void
collision_detection(void)
{
    int i, j;
    Collision *c, *c_tmp, fubar = {0};

    if (g_collision_obj < 2)
        return;

    c_list = malloc(sizeof(Collision) * (g_collision_obj + 1));
    if (!c_list)
        panic("Out of memory.");

    c = c_base;
    for (i = 0; i < g_collision_obj; i++) {
        c_list[i] = c;
        c = c->next;
    }

    /* XXX: The sort function goes out of its sorting range by one.
     */

    c_list[i] = &fubar;
    sort_list(c_list, g_collision_obj);

    for (i = 0; i < (g_collision_obj - 1); i++) {
        for (j = i + 1; j < g_collision_obj; j++) {
            if (c_list[i]->x1 < c_list[j]->x0)
                break;

            if (c_list[i]->y0 <= c_list[j]->y1 &&
                c_list[i]->y1 >= c_list[j]->y0) {
                if (!c_list[i]->pend_rm) {
                    c_list[i]->pend_rm =
                        c_list[i]->handler(c_list[i], c_list[j]);
                }
                if (!c_list[j]->pend_rm) {
                    c_list[j]->pend_rm =
                        c_list[j]->handler(c_list[j], c_list[i]);
                }
            }
        }
    }

    c = c_base;
    while (c) {
        c_tmp = c;
        c = c->next;
        if (c_tmp->pend_rm)
            collision_destroy(c_tmp);
    }

    free(c_list);
}
