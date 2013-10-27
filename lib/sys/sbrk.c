/*
 * get memory from the OS
 * written by Eric R. Smith, placed in the public domain
 * FIXME: should do error checking based on the stack; but
 * that is tricky if threads alloc their memory from the stack
 * for now, use the stack of the first thread to request memory
 */

#include <compiler.h>

extern char __heap_start[];
extern char __hub_heap_start[];

#if defined(__PROPELLER_LMM__) || defined(__PROPELLER_XMMC__) || defined(__PROPELLER_CMM__)
#define HUB_SBRK_ONLY
#else
char *_heap_base = __heap_start;
#endif

char *_hub_heap_base = __hub_heap_start;
char *_hub_heap_end = 0;

#define MIN_STACK 128

char *
_hubsbrk(unsigned long n)
{
  char *r = _hub_heap_base;
  char *here = (char *)&r;

  if (here > _hub_heap_end) {
    if (_hub_heap_end == 0)
      _hub_heap_end = here - MIN_STACK;
    if (r + n > _hub_heap_end)
      return 0; /* not enough memory */
  }
  /* allocate and return */
  _hub_heap_base = r + n;
  return r;
}

#if defined(HUB_SBRK_ONLY)
/* hubsbrk is just the same as sbrk */
__weak_alias(_sbrk, _hubsbrk);
#else
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
#endif

/* for porting C programs */
__weak_alias(sbrk, _sbrk);
