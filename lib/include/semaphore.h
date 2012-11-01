#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct sem_t {
  volatile int counter;
  _pthread_queue_t queue;
} sem_t;

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif
