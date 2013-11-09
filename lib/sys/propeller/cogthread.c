#include <propeller.h>
#include <sys/thread.h>
#include <errno.h>

#if defined(__PROPELLER_USE_XMM__)
// the default cache size to use for new threads:
// 32 lines of 32 bytes each
#define INDEX_BITS 5
#define OFFSET_BITS 5
#define CACHE_SIZE 1024
#define CACHE_GEOMETRY ((INDEX_BITS<<8) | (OFFSET_BITS) )
extern unsigned int _hub_mailbox __asm__("xmem_hubaddrp") __attribute__((cogmem));
#endif

extern void _clone_cog(void *tmp);

/*
 * start C code running in another cog
 * returns -1 on failure, otherwise the
 * id of the new cog
 * "func" is the function to start running
 * "arg" is the argument
 * "stacktop" is the top of the new process' stack
 * NOTE: this is a raw low-level function; the
 * pthreads functions may be more useful
 */
int
_start_cog_thread(void *stacktop, void (*func)(void *), void *arg, _thread_state_t *tls)
{
  void *tmp = __builtin_alloca(1984);
  unsigned int *sp = stacktop;
  int r;

  /* copy the kernel into temporary (HUB) memory */
  _clone_cog(tmp);

  /* push the pointer to thread local storage */
  *--sp = (unsigned int)tls;

  /* push the parameter to the function */
  *--sp = (unsigned int)arg;

  /* push the code address */
  *--sp = (unsigned int)func;

#if defined(__PROPELLER_USE_XMM__)
  {
    unsigned int cache_size = CACHE_SIZE;
    unsigned int tag_size = (1<<(INDEX_BITS+2));
    unsigned char *hubdata;
    unsigned int cogid = __builtin_propeller_cogid();

    {
      // FIXME: we need to free this memory if the thread
      // exits!!
      extern char *_hubsbrk(unsigned long);
      hubdata = (unsigned char *)_hubsbrk(cache_size+tag_size+15);
      hubdata = (unsigned char *)(~0xf & (15 + (unsigned)hubdata));
      memset(hubdata, 0, cache_size+tag_size);
    }
    // push the cache geometry
    *--sp = CACHE_GEOMETRY;
    // push the cache line storage address
    *--sp = (unsigned int)hubdata;
    // push the cache tag storage address
    *--sp = (unsigned int)(hubdata + cache_size);
    // push the base of the hub mailbox array
    *--sp = ((unsigned int)_hub_mailbox) - (cogid<<3);
  }
#endif
  /* now start the kernel */
  r = cognew(tmp, sp);

  return r;
}
