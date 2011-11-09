#include <pthread.h>

int
pthread_detach(pthread_t thr)
{
  _thread_state_t *thread = _pthread_ptr(thr);

  if (!thread)
    return -1;
  __lock_pthreads();
  if (thread->flags & _PTHREAD_TERMINATED)
    _pthread_free(thread);
  else
    thread->flags |= _PTHREAD_DETACHED;
  __unlock_pthreads();
  return 0;
}
