#include <pthread.h>
#include <stdio.h>
#include <sys/driver.h>

/*
 * use the full duplex serial driver
 */
extern _Driver _FullDuplexSerialDriver;

_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  NULL
};

#define NUM_THREADS 16
pthread_t thr[NUM_THREADS];

volatile int stdio_lock;

void *
threadfunc(void *arg)
{
  int n = (int)arg;
  int i;

  _lock(&stdio_lock);
  printf("in thread %d\n", n);
  fflush(stdout);
  _unlock(&stdio_lock);
  for (i = 0; i < 10; i++) {
    _lock(&stdio_lock);
    printf("hello %d from thread %d\n", i, n);
    fflush(stdout);
    _unlock(&stdio_lock);
    pthread_yield();
  }
  return (void *)(n*n);
}

int
main()
{
  int i;
  int r;

  printf("threads demo\n");
  fflush(stdout);
  for (i = 0; i < NUM_THREADS; i++) {
    int name = i;
    _lock(&stdio_lock);
    printf("creating thread %d\n", name);
    fflush(stdout);
    _unlock(&stdio_lock);
    r = pthread_create(&thr[i], NULL, threadfunc, (void *)(name));
  }
  for (i = 0; i < NUM_THREADS; i++) {
    void *result;
    r = pthread_join(thr[i], &result);
    _lock(&stdio_lock);
    if (r != 0)
      printf("error in pthread_join on thread %d\n", i);
    else
      printf("thread %d returned %d\n", i, (int)result);
    _unlock(&stdio_lock);
  }
  printf("all done\n");
  return 0;
}
