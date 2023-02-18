#pragma once

#include <inttypes.h>

#include "sprite.h"

typedef struct _Collision Collision;

struct _Collision {
    /* ID data
     */
    int id; /* ID number and ID pointer */
    void* id_p;
    int gid;     /* Group ID */
    int pend_rm; /* Used by collision routine */

    /* Called upon collision.
     */
    int (*handler)(Collision*, Collision*);

    /* Udated when ploting the object.
     */
    int x0; /* Left boundary */
    int x1; /* Right boundary */
    int y0; /* Upper boundary */
    int y1; /* Lower boundary */

    /* Used if blocks != 0.
     */
    int blocks;    /* Width of mask in int32_ts */
    uint32_t* mask; /* One bit collision mask */

    Collision* prev;
    Collision* next;
};

extern int g_collision_obj;

Collision* collision_create(int id, void* id_p, int gid,
                            int (*handler)(Collision*, Collision*));

void collision_destroy(Collision* c);
void collision_update_from_sprite(Collision* c, Sprite* s);
void collision_detection(void);
