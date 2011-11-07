#include <pthread.h>

int
pthread_detach(pthread_t thr)
{
  _pthread_status_t *thread = _pthread_ptr(thr);

  if (!thread)
    return -1;
  _lock_pthreads();
  if (thread->flags & _PTHREAD_TERMINATED)
    thread->flags = 0;  /* free it if already terminated */
  else
    thread->flags |= _PTHREAD_DETACHED;
  _unlock_pthreads();
  return 0;
}
