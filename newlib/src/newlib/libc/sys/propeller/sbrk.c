/*
 * get memory from the OS
 * based on original by Eric R. Smith, placed in the public domain
 * FIXME: should do error checking based on the stack
 */

#include <reent.h>

extern char __heap_start[];

char *_heap_base = __heap_start;

#define MIN_STACK 128

void *
_sbrk_r(struct _reent *reent, ptrdiff_t n)
{
  char *here;
  char c;
  char *r = _heap_base;
  here = &c;

  /* check that there will still be stack left after */
  if ((long)(here - (r+n)) < MIN_STACK)
    return (void *)0;

  /* allocate and return */
  _heap_base = r + n;
  return (void *)r;
}
