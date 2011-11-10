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

#define LOOPCOUNT 10
#define SLEEPTIME 3
#define NUM_THREADS 11
pthread_t thr[NUM_THREADS];

void *
threadfunc(void *arg)
{
  int n = (int)arg;
  int i;

  printf("in thread %d\n", n);
  for (i = 0; i < LOOPCOUNT; i++) {
    printf("hello %d from thread %d (on cog %d)\n", i, n, __builtin_propeller_cogid());
#if SLEEPTIME > 0
    sleep(SLEEPTIME);
#else
    pthread_yield();
#endif
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
  printf("again\n");
  for (i = 0; i < NUM_THREADS; i++) {
    int name = i;
    printf("creating thread %d\n", name);
    fflush(stdout);
    r = pthread_create(&thr[i], NULL, threadfunc, (void *)(name));
  }
  for (i = 0; i < NUM_THREADS; i++) {
    void *result;
    r = pthread_join(thr[i], &result);
    if (r != 0)
      printf("error in pthread_join on thread %d\n", i);
    else
      printf("thread %d returned %d\n", i, (int)result);
  }
  printf("all done\n");
  return 0;
}
