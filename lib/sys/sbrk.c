/*
 * get memory from the OS
 * written by Eric R. Smith, placed in the public domain
 * FIXME: should do error checking based on the stack; but
 * that is tricky if threads alloc their memory from the stack
 */

#include <compiler.h>

extern char __heap_start[];
extern char __hub_heap_start[];

#if defined(__PROPELLER_LMM__) || defined(__PROPELLER_XMMC__) || defined(__PROPELLER_CMM__)
char *_heap_base = __hub_heap_start;
#define NO_HUB_SBRK
#else
char *_heap_base = __heap_start;
char *_hub_heap_base = __hub_heap_start;
#endif

#define MIN_STACK 128

char *
_sbrk(unsigned long n)
{
  //char c;
  //char *here = &c;
  char *r = _heap_base;

  /* allocate and return */
  _heap_base = r + n;
  return r;
}

#if defined(NO_HUB_SBRK)
/* hubsbrk is just the same as sbrk */
__weak_alias(_hubsbrk, _sbrk);
#else
char *
_hubsbrk(unsigned long n)
{
  //char c;
  //char *here = &c;
  char *r = _hub_heap_base;

  /* allocate and return */
  _hub_heap_base = r + n;
  return r;
}
#endif

/* for porting C programs */
__weak_alias(sbrk, _sbrk);
