#define RUNLEVEL_TITLE0  0   /* Draw title */
#define RUNLEVEL_TITLE1  1   /* Clear title */

#define RUNLEVEL_PLAY0   3   /* Draw bombers */
#define RUNLEVEL_PLAY1   4   /* ... */
#define RUNLEVEL_PLAY2   5   /* Clear bombers */


typedef struct _RunLevelFunc RunLevelFunc;

struct _RunLevelFunc
{
  int from;             /* 'from' runlevel 'to' runlevel */
  int to;
  int (*handler)(RunLevelFunc *);
  int rval;             /* Return value from handler(); 1 -> OK to switch */
  RunLevelFunc *prev;
  RunLevelFunc *next;
};

extern int g_runlevel;
extern int g_next_runlevel;

void
runlevel_update ();

void
runlevel_register_func (RunLevelFunc *r, int from, int to,
			int (*handler)(RunLevelFunc *));
