#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include "all.h"

static PanicCleanUp* base = 0;

/* Print warning message.
 */

void
warning(char* format, ...)
{
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "W: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

/* Call cleanup handlers, print message and exit.
 */

void
panic(char* format, ...)
{
    va_list ap;
    PanicCleanUp* p;

    p = base;
    while (p) {
        p->handler();
        p = p->next;
    }

    va_start(ap, format);
    fprintf(stderr, "E: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(1);
}

/* Register clean-up handler.
 */

void
panic_register_cleanup(PanicCleanUp* p)
{
    if (base) {
        p->next = base;
        p->next->prev = p;
    } else {
        p->next = 0;
    }

    p->prev = 0;
    base = p;
}
