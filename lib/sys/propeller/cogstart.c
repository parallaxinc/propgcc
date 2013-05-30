#include <propeller.h>
#include <sys/thread.h>
#include <errno.h>

/* assembly function that copies COG internal memory to
   a buffer
   the buffer needs to be bit enough for the RAM
   (0x7C0, or 1984 bytes)
*/
extern void _clone_cog(void *buf);

/*
 * start C code running in another cog
 * returns -1 on failure, otherwise the
 * id of the new cog
 * "func" is the function to start running
 * "arg" is the argument
 * "stack" is the base of the new process' stack
 * "stack_size" is the size of the stack area in bytes
 * NOTE: this is a raw low-level function; the
 * pthreads functions may be more useful
 */
int
cogstart(void (*func)(void *), void *arg, void *stack, size_t stack_size)
{
#if (defined(__PROPELLER_LMM__) || defined(__PROPELLER_CMM__))
  _thread_state_t *tls;
  unsigned int *sp;
  void *tmp;

  /* check the stack size */
  if (stack_size < sizeof(_thread_state_t) + 3 * sizeof(unsigned int)) {
    errno = EINVAL;
    return -1;
  }
  
  /* push the thread local storage structure onto the stack */
  tls = (_thread_state_t *)((char *)stack + stack_size - sizeof(_thread_state_t));
  sp = (unsigned int *)tls;

  /* push the pointer to thread local storage */
  *--sp = (unsigned int)tls;

  /* push the parameter to the function */
  *--sp = (unsigned int)arg;

  /* push the code address */
  *--sp = (unsigned int)func;

  /* allocate enough space on the caller's stack for the kernel image */
  /* this will be in HUB memory */
  tmp = __builtin_alloca(1984);  
  _clone_cog(tmp);

  /* now start the kernel */
  return cognew(tmp, sp);
#else
  errno = ENOSYS;
  return -1;
#endif
}
