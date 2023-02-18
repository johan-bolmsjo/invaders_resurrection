typedef struct _Shot Shot;

struct _Shot {
    int x;
    int y;
    int x_vector;
    int y_vector;
    int16_t colour;
    uint8_t pend_rm; /* Pending removal if set */
    int16_t* adr[2]; /* Plot adresses */
    Collision* c;    /* Collision callback function */
    void (*cb)();    /* Extra callback function for player shots */
    Shot* prev;
    Shot* next;
};

extern int g_shot_obj;

Shot* shot_create(int x, int y, int x_vector, int y_vector, int16_t colour,
                  int fatal, int gid, void (*cb)());

void shot_hide(DG* dg);
void shot_show(DG* dg);
void shot_update();
