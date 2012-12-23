#include <propeller.h>
#include <sys/thread.h>
#include <errno.h>

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
#if !defined(__PROPELLER2__) && (defined(__PROPELLER_LMM__) || defined(__PROPELLER_CMM__))
  extern unsigned int _load_start_kernel[];
  unsigned int *sp = stacktop;

  /* push the pointer to thread local storage */
  *--sp = (unsigned int)tls;

  /* push the parameter to the function */
  *--sp = (unsigned int)arg;

  /* push the code address */
  *--sp = (unsigned int)func;

  /* now start the kernel */
  return cognew(_load_start_kernel, sp);
#else
  errno = ENOSYS;
  return -1;
#endif
}
