#include <pthread.h>
#include <errno.h>
#include <string.h>

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  memset(mutex, 0, sizeof(mutex));
  return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  return 0;
}

/*
 * try to lock a mutex
 * the mutex variable cnt is a count of how many threads want this mutex
 * if it is 0, the mutex is free
 * if it is >0, someone owns the mutex
 */
int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  return (__sync_bool_compare_and_swap(&mutex->cnt, 0, 1)) ? 0 : EBUSY;
}

/*
 * lock a mutex
 * we increment the mutex want count; if it is now 1, we're done, otherwise
 * we have to wait for it
 */
int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  if (_addlock(&mutex->cnt, 1) == 1)
    {
      return 0;
    }

  /* failed to get the mutex */
  /* sleep on the queue */
  do {
    _pthread_sleep(&mutex->queue);
  } while (0 != pthread_mutex_trylock(mutex));
  return 0;
}

/*
 * release the mutex
 * we atomically decrement the count of threads that want this
 * mutex; if it goes from 1->0 then we are done, otherwise we
 * have to wait one of the threads that was sleeping on the queue
 */
int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  if (_addlock(&mutex->cnt, -1) == 0)
    {
      return 0;
    }
  /* someone else was sleeping on the queue
     (or is about to... watch out for the race condition
     where this function is called during another thread's
     attempt to lock the mutex)
  */
  while (!_pthread_wake(&mutex->queue))
    pthread_yield();

  return 0;
}
