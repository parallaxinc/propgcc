/*
 * @pthread.c
 * Implementation of pthread functions
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

/*
 * pthread scheduler
 */

#define NDEBUG  /* turn off asserts */
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <cog.h>
#include <propeller.h>

/* define REAL_SLEEP to add a timer queue that processes can sleep on
 * if that isn't defined, sleeps will just be busy waits
 */
#define REAL_SLEEP

/* the ready queue are threads that are ready to run but have no cog assigned
 * for them yet
 * the join queue are threads that are waiting on some other thread to terminate
 */
_pthread_queue_t __ready_queue;
_pthread_queue_t __join_queue;

#ifdef REAL_SLEEP
_pthread_queue_t __timer_queue;
#endif
atomic_t __pthreads_lock;

#define ASSERT_LOCKED() assert(__pthreads_lock != 0)

/* destructors for thread specific data */
__pthread_destruct_func __pdestruct[PTHREAD_KEYS_MAX];

/* this should be called only if we already hold the _pthread_lock */
static void
_pthread_addqueue(_pthread_queue_t *queue, _pthread_state_t *thr)
{
  _pthread_state_t *p;

  ASSERT_LOCKED();

  thr->queue = queue;
  for(;;) {
    p = *queue;
    if (!p || thr->pri > p->pri)
      break;
    queue = &p->queue_next;
  }
  thr->queue_next = *queue;
  *queue = thr;
}

/* this should be called only if we already hold the _pthread_lock */
/* get the first thing in the queue that is allowed on this cog
   (has proper affinity)
*/
static _pthread_state_t *
_pthread_getqueuehead_affinity(_pthread_queue_t *queue)
{
  _pthread_state_t *p = *queue;
  unsigned short cpumask = __this_cpu_mask();
  ASSERT_LOCKED();
  while (p && (p->affinity & cpumask) != 0)
    {
      queue = &p->queue_next;
      p = *queue;
    }

  if (p) {
    *queue = p->queue_next;
    p->queue_next = NULL;
    p->queue = NULL;
  }
  return p;
}

/* get the first thing in the queue regardless of lock
   (has proper affinity)
*/
static _pthread_state_t *
_pthread_getqueuehead(_pthread_queue_t *queue)
{
  _pthread_state_t *p = *queue;
  ASSERT_LOCKED();
  if (p) {
    *queue = p->queue_next;
    p->queue_next = NULL;
    p->queue = NULL;
  }
  return p;
}

/*
 * main pthreads scheduler functions
 * call these with the pthread_lock being
 * held
 */
static void
_pthread_schedule_raw(void)
{
  _pthread_state_t *next;

 again:
  ASSERT_LOCKED();
#if defined(REAL_SLEEP)
  if (__timer_queue) {
    /* wake up everything on the timer queue whose
       timer limit has elapsed
     */
    unsigned int now = getcnt();
    next = __timer_queue;
    while (next && (int)(next->timer - now) <= 0)
      {
	assert(next != next->queue_next);
	__timer_queue = next->queue_next;
	next->queue_next = NULL;
	next->queue = NULL;
	_pthread_addqueue(&__ready_queue, next);
	next = __timer_queue;
      }
  }
#endif
  if (__ready_queue) {
    next = _pthread_getqueuehead_affinity(&__ready_queue);
    if (next) {
      _TLS = next;
      longjmp(next->jmpbuf, 1);
    }
  }
  /* nothing ready to run */
  /* hmmm, this is a tricky one; let's hope something on another CPU
     will eventually add something to the ready queue
     release the pthreads lock and wait for that
  */
  __unlock_pthreads();
#if defined(REAL_SLEEP)
  while (!__ready_queue && !__timer_queue) 
    {
    }
#else
  while (!__ready_queue) 
    {
    }
#endif
  __lock_pthreads();
  goto again;
}
static void
_pthread_schedule(void)
{
  _pthread_state_t *self = _pthread_self();
  ASSERT_LOCKED();
  if (setjmp(self->jmpbuf) == 0) {
    _pthread_schedule_raw();
  }
}

/*
 * go to sleep on a queue
 */
void
_pthread_sleep_with_lock(_pthread_queue_t *queue)
{
  ASSERT_LOCKED();
  _pthread_addqueue(queue, _pthread_self());
  _pthread_schedule();
}

void
_pthread_sleep(_pthread_queue_t *queue)
{
  __lock_pthreads();
  _pthread_sleep_with_lock(queue);
  __unlock_pthreads();
}

#if defined(REAL_SLEEP)
/*
 * put a thread to sleep until the system clock reaches or
 * passes "newclock"
 */

static void
_pthread_napuntil(unsigned int newclock)
{
  _pthread_state_t *thr = _pthread_self();
  _pthread_state_t *p;
  _pthread_queue_t *queue = &__timer_queue;
  unsigned int now;
  int delta;

  __lock_pthreads();
  /* add the current thread to the "timer" queue
     that queue is kept sorted by time to wake up
  */
  now = getcnt();;
  thr->timer = newclock;
  delta = (int)(thr->timer - now);
    {
      thr->queue = queue;
      for(;;) {
	p = *queue;
	if (!p || ((int)(p->timer-now) > delta))
	    break;
	queue = &p->queue_next;
      }
      assert(*queue != thr);
      assert(thr->queue_next == 0);
      thr->queue_next = p;
      *queue = thr;
      _pthread_schedule();
    }
  __unlock_pthreads();
}
#endif

/*
 * yield the processor to another thread
 */
void
pthread_yield(void)
{
    _pthread_sleep(&__ready_queue);
}

/*
 * wake the highest priority process waiting on a queue
 * returns a count of threads woken
 */
int
_pthread_wake(_pthread_queue_t *queue)
{
  int cnt = 0;
  _pthread_state_t *head;
  __lock_pthreads();
  head = _pthread_getqueuehead(queue);
  if (head) {
    cnt++;
    _pthread_addqueue(&__ready_queue, head);
  }
  __unlock_pthreads();
  if (cnt) {
    pthread_yield(); // allow the new thread to run if it is higher priority
  }
  return cnt;
}

/*
 * wake all threads waiting on a queue
 * returns a count of threads woken
 */
int
_pthread_wakeall(_pthread_queue_t *queue)
{
  int cnt = 0;
  _pthread_state_t *head;
  __lock_pthreads();
  for(;;) {
    head = _pthread_getqueuehead(queue);
    if (!head) break;
    _pthread_addqueue(&__ready_queue, head);
    cnt++;
  }
  __unlock_pthreads();
  if (cnt > 0)
    {
      pthread_yield(); // allow the new thread to run if it is higher priority
    }
  return cnt;
}

void
_pthread_free(_pthread_state_t *thr)
{
  thr->flags = 0;
  free(thr);
}

/*********************************************
 * the initial entry point for threads
 *********************************************/
static void
_pthread_main(void *dummy)
{
  void *r;
  _pthread_state_t *thr = _pthread_self();
  r = (*thr->start)(thr->arg);
  pthread_exit(r);
}

/*********************************************
 * now the actual thread creation code
 *********************************************/

int
pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	       void *(startfunc)(void*), void *arg)
{
  _pthread_state_t *thr;
  size_t datasize;
  size_t stksiz;
  unsigned char *stack;
  int cog;

  /* make sure the library yield() hook points to the right thing */
  __yield_ptr = pthread_yield;
#if defined(REAL_SLEEP)
  __napuntil_ptr = _pthread_napuntil;
#endif

  /* set up the new thread */
  datasize = sizeof(*thr);
  if (attr == NULL)
    {
      /* use default values */
      stksiz = 0;
      stack = 0;
    }
  else
    {
      stksiz = attr->stksiz;
      stack = attr->stack;
    }

  if (stksiz == 0)
    stksiz = _PTHREAD_STACK_DEFAULT;  /* default stack size */
  else
    {
      stksiz = (stksiz + 3) & ~3; /* round up to nearest word */
      if (stksiz < PTHREAD_STACK_MIN)
	{
	  errno = EINVAL;
	  return -1;
	}
    }
  if (stack == 0) {
    datasize += stksiz;
  }
  thr = _hubcalloc(1, datasize);

  if (!thr)
    return -1;
  if (!stack)
    stack = ((unsigned char *)thr) + sizeof(*thr);

  /* move to top of stack */
  stack = stack + stksiz;

  *thread = thr;
  thr->arg = arg;
  thr->start = startfunc;

  /* now we have to actually get the thread started */
  /* first try to start it on another cog */
  cog = _start_cog_thread(stack, _pthread_main, 0, thr);

  if (cog < 0)
    {
      _pthread_state_t *self = _pthread_self();
      /* we'll have to run it on this processor */
      /* set up the jmp_buf so that it will return to _pthread_main */
      thr->jmpbuf[_REG_LR] = (unsigned long)_pthread_main;
      thr->jmpbuf[_REG_SP] = (unsigned long)stack;

      __lock_pthreads();

      /* save the current thread's state */
      if (setjmp(self->jmpbuf) == 0) {
	_pthread_addqueue(&__ready_queue, self);
	_TLS = thr;
	__unlock_pthreads();
	longjmp(thr->jmpbuf, 1);
      }
      __unlock_pthreads();
    }

  /* indicate everything is OK */
  return 0;
}

void
pthread_exit(void *result)
{
  _pthread_state_t *self = _pthread_self();
  void *val;
  int i;
  int calledone;

  self->arg = result;
  /* call destructors */
  /* we have to keep doing this until all key values are NULL;
     destructors are allowed to set keys
  */
  do {
    calledone = 0;
    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
      val = pthread_getspecific(i);
      if (val && __pdestruct[i]) {
	pthread_setspecific(i, NULL);
	(*__pdestruct[i])(val);
	calledone = 1;
      }
    }
  } while (calledone > 0);

  __lock_pthreads();
  if (self->flags & _PTHREAD_DETACHED)
    {
      /* we have to free the memory allocated to this thread, and
	 start executing a new thread */
      _pthread_free(self);
      /* now go run some other thread */
      _pthread_schedule_raw();
    }
  self->flags |= _PTHREAD_TERMINATED;
  __unlock_pthreads();

  /* wake up anyone waiting for us */
  _pthread_wakeall(&__join_queue);
  for(;;) {
    __lock_pthreads();
    _pthread_schedule_raw();
  }
}

pthread_t
pthread_self(void)
{
  return _pthread_self();
}

// magic to pull in the _driverlist from pthread_io.c to force FullDuplexSerial
__asm__ (" .global __pthreadDriverList\n .set __pthreadDriverList, __driverlist\n");

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * +--------------------------------------------------------------------
 */
