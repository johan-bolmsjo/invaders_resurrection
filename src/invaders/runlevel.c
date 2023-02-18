/* Support code to more easily handle run level changes in the game.
 * By run level I mean for example quiting the game and go back to
 * the title screen. Many events needs to be checked and taken action
 * on before a change can be made.
 */

#include "runlevel.h"

#include <inttypes.h>

static RunLevelFunc* base = 0;

int g_runlevel = RUNLEVEL_TITLE0;      /* Current runlevel */
int g_next_runlevel = RUNLEVEL_TITLE0; /* Next runlevel */

/* Change runlevel if all agree.
 */

void
runlevel_update()
{
    int change = 1;
    RunLevelFunc* r;

    if (g_runlevel != g_next_runlevel) {
        r = base;
        while (r) {
            if (r->from == g_runlevel &&
                r->to == g_next_runlevel) {
                if (!r->rval)
                    r->rval = r->handler(r);
                if (!r->rval)
                    change = 0;
            }
            r = r->next;
        }

        if (change) {
            r = base;
            while (r) {
                r->rval = 0;
                r = r->next;
            }

            g_runlevel = g_next_runlevel;
        }
    }
}

/* Register run level function to be executed to check wether a
 * condition has been met.
 */

void
runlevel_register_func(RunLevelFunc* r, int from, int to,
                       int (*handler)(RunLevelFunc*))
{
    r->from = from;
    r->to = to;
    r->handler = handler;
    r->rval = 0;

    if (base) {
        r->next = base;
        r->next->prev = r;
    } else {
        r->next = 0;
    }

    r->prev = 0;
    base = r;
}
