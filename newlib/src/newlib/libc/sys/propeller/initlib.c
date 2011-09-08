#include <errno.h>
#include <reent.h>

extern struct _reent *_impure_ptr;  /* in reent.c */
KTMUTEX *alloc_mutex;

void
__initlib(void)
{
    /* do any other setup that's required */
}

