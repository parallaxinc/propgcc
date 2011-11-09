/*
 * pthread scheduler
 *
 */
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>

_pthread_queue_t __ready_queue;
_pthread_queue_t __join_queue;

atomic_t __pthreads_lock;

/* this should be called only if we already hold the _pthread_lock */
static void
_pthread_addqueue(_pthread_queue_t *queue, _pthread_state_t *thr)
{
  _pthread_state_t *p;

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
static _pthread_state_t *
_pthread_getqueuehead(_pthread_queue_t *queue)
{
  _pthread_state_t *p = *queue;
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
  if (__ready_queue) {
    next = _pthread_getqueuehead(&__ready_queue);
    _TLS = next;
    longjmp(next->jmpbuf, 1);
  } else {
    /* nothing ready to run */
    /* hmmm, this is a tricky one; let's hope something on another CPU
       will eventually add something to the ready queue
       release the pthreads lock and wait for that
    */
    __unlock_pthreads();
    while (!__ready_queue) ;
    __lock_pthreads();
    goto again;
  }
}
static void
_pthread_schedule(void)
{
  _pthread_state_t *self = _pthread_self();
  if (setjmp(self->jmpbuf) == 0) {
    _pthread_schedule_raw();
  }
}

/*
 * go to sleep on a queue
 */
void
_pthread_sleep(_pthread_queue_t *queue)
{
  __lock_pthreads();
  _pthread_addqueue(queue, _pthread_self());
  _pthread_schedule();
  __unlock_pthreads();
}

void
_pthread_sleep_with_lock(_pthread_queue_t *queue)
{
  _pthread_addqueue(queue, _pthread_self());
  _pthread_schedule();
}

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
    _pthread_schedule();
  }
  __unlock_pthreads();
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
  if (cnt > 0)
    _pthread_schedule();
  __unlock_pthreads();
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
    stksiz = 512;  /* default stack size */
  else
    {
      stksiz = (stksiz + 3) & ~3; /* round up to nearest word */
      if (stksiz < 16)
	{
	  errno = EINVAL;
	  return -1;
	}
    }
  if (stack == 0) {
    datasize += stksiz;
  }
  thr = calloc(1, datasize);
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
  self->arg = result;
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
  _pthread_schedule_raw();
}

pthread_t
pthread_self(void)
{
  return _pthread_self();
}
