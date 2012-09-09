#include <semaphore.h>
#include <errno.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
  if ((int)value < 0)
    return EINVAL;
  sem->counter = value;
  return 0;
}

int sem_destroy(sem_t *sem)
{
  return 0;
}

int sem_post(sem_t *sem)
{
  __lock_pthreads();
  sem->counter += 1;
  if (sem->counter <= 0) {
    __unlock_pthreads();
    _pthread_wake(&sem->queue);
  } else {
    __unlock_pthreads();
  }
 return 0;
}

int sem_wait(sem_t *sem)
{
  __lock_pthreads();
  sem->counter -= 1;
  if (sem->counter < 0) {
    _pthread_sleep_with_lock(&sem->queue);
  }
  __unlock_pthreads();
  return 0;
}

int sem_trywait(sem_t *sem)
{
  int result;
  int ret;

  __lock_pthreads();
  result = sem->counter-1;
  if (result >= 0) {
    sem->counter = result;
    ret = 0;
  } else {
    errno = EAGAIN;
    ret = -1;
  }
  __unlock_pthreads();
  return ret;
}
