#include "runlevel.h"

#include <inttypes.h>

static RunLevelFunc* base = 0;

int g_runlevel      = RUNLEVEL_TITLE0; // Current runlevel
int g_next_runlevel = RUNLEVEL_TITLE0; // Next runlevel

void
runlevel_update(void)
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
