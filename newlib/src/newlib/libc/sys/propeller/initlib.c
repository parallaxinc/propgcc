#include <errno.h>
#include <reent.h>

extern struct _reent *_impure_ptr;  /* in reent.c */

void
__initlib(void)
{
    /* do any other setup that's required */
}

