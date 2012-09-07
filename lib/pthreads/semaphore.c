#include <semaphore.h>
#include <errno.h>

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
  pthread_cond_init(&sem->cond, NULL);
  pthread_mutex_init(&sem->mutex, NULL);
  sem->counter = value;
  return 0;
}

int sem_destroy(sem_t *sem)
{
  pthread_cond_destroy(&sem->cond);
  pthread_mutex_destroy(&sem->mutex);
  return 0;
}

int sem_post(sem_t *sem)
{
  int result = __addlock(&sem->counter, 1);
  if (result == 0) {
    pthread_cond_signal(&sem->cond);
  } 
 return 0;
}

int sem_wait(sem_t *sem)
{
  int result;

  result = __addlock(&sem->counter, -1);
  if (result < 0) {
    pthread_mutex_lock(&sem->mutex);
    for(;;) {
      if (sem->counter >= 0) break;
      pthread_cond_wait(&sem->cond, &sem->mutex);
    }
    pthread_mutex_unlock(&sem->mutex);
  }
  return 0;
}

int sem_trywait(sem_t *sem)
{
  int result;

  result = __addlock(&sem->counter, -1);
  if (result < 0) {
    __addlock(&sem->counter, 1);
    errno = EAGAIN;
    return -1;
  }
  return 0;
}
