typedef struct _Player {
    int count;
    Sprite s;
    Collision* c;
} Player;

extern int g_player_alive;

void player_tables(Joy* joy);
void player_kill();
void player_anim(Player* p, Joy* j);
void player_move(Player* p);
void player_hide(DG* dg);
void player_show(DG* dg);
void player_update(Joy* j, int* key_q);
