/*
 * get memory from the OS
 * written by Eric R. Smith, placed in the public domain
 * FIXME: should do error checking based on the stack
 */

extern char __heap_start[];

char *_heap_base = __heap_start;

#define MIN_STACK 1024

char *
_sbrk(unsigned long n)
{
  char *here;
  char c;
  char *r = _heap_base;
  here = &c;

  /* check that there will still be stack left after */
  if ((long)(here - (r+n)) < MIN_STACK)
    return (char *)0;

  /* allocate and return */
  _heap_base += n;
  return r;
}
