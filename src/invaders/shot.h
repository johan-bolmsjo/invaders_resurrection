#pragma once
/// \file shot.h
///
/// Player shots and collision particles.
///

#include <inttypes.h>

#include "collision.h"

typedef struct _Shot Shot;

struct _Shot {
    int x;
    int y;
    int x_vector;
    int y_vector;
    uint16_t colour;
    uint8_t pend_rm;  /* Pending removal if set */
    uint16_t* adr[2]; /* Plot adresses */
    Collision* c;     /* Collision callback function */
    void (*cb)(void); /* Extra callback function for player shots */
    Shot* prev;
    Shot* next;
};

extern int g_shot_obj;

/// Create shot.
Shot* shot_create(int x, int y, int x_vector, int y_vector, uint16_t colour,
                  int fatal, int gid, void (*cb)(void));


/// Hide all shots.
void shot_hide(const DG* dg);

/// Show all shots.
void shot_show(const DG* dg);

/// Update all shots.
void shot_update(void);
