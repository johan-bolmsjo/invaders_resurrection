typedef struct _Ufo {
    int count;
    int dir;
    Sprite s;
    Collision* c;
} Ufo;

void ufo_tables();
void ufo_init(Ufo* u, int x, int y, int vis);
void ufo_anim(Ufo* u);
