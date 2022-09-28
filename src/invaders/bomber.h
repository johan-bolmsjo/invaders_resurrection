typedef struct _Bomber
{
  int count;
  int x, y;   /* Used in armada.c */
  Sprite s;
  Collision *c;
} Bomber;


void
bomber_init (Bomber *b);

void
bomber_anim (Bomber *b);
