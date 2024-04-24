#pragma once
/// \file collision.h
///
/// Collision detection.
///

#include <inttypes.h>

#include "sprite.h"

struct Collision;

struct Collision {
    // ID data =>

    int   id;                   // ID number and ID pointer
    void* id_p;
    int   gid;                  // Group ID
    int   pend_rm;              // Used by collision routine

    // Invoked on collision between two objects.
    int (*handler)(struct Collision*, struct Collision*);

    //  Udated when ploting the object =>

    int x0;                     // Left boundary
    int x1;                     // Right boundary
    int y0;                     // Upper boundary
    int y1;                     // Lower boundary

    // Used if blocks != 0 =>

    int       blocks;           // Width of mask in int32_ts
    uint32_t* mask;             // One bit collision mask

    struct Collision* prev;
    struct Collision* next;
};

extern int g_collision_obj;

/// Creates collision object with a callback function that is called upon collision.
struct Collision* collision_create(int id, void* id_p, int gid,
                                   int (*handler)(struct Collision*, struct Collision*));

/// Removes a collition object.
void collision_destroy(struct Collision* c);

/// Uppdate a collition object with data from a sprite.
void collision_update_from_sprite(struct Collision* c, struct Sprite* s);

/// Scans collision object list and calls call-back functions upon collision.
void collision_detection(void);
