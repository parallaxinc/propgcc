/*
 * get memory from the OS
 * written by Eric R. Smith, placed in the public domain
 * FIXME: should do error checking based on the stack; but
 * that is tricky if threads alloc their memory from the stack
 */

extern char __heap_start[];

char *_heap_base = __heap_start;

#define MIN_STACK 128

char *
_sbrk(unsigned long n)
{
  char *here;
  char c;
  char *r = _heap_base;
  here = &c;

  /* allocate and return */
  _heap_base = r + n;
  return r;
}
