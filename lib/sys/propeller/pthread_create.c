/*
 * @pthread.c
 * Implementation of pthread functions
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
#include <pthread.h>
#include <errno.h>
#include <propeller.h>
#include <stdlib.h>

_pthread_status_t _PTHREAD[_NUM_PTHREADS];

/* lock needed for pthreads */
volatile int _pthreads_lock;

/* function to actually start a pthread running */
static void
pthread_run(void *ptr)
{
  void *result;
  _pthread_status_t *p = ptr;

  result = (*p->startfunc)(p->arg);
  pthread_exit(result);
}

/* create a new pthread */
int
pthread_create(pthread_t *threadId_ptr,
	       const pthread_attr_t *attr,
	       void *(*startfunc)(void *),
	       void *arg)
{
  void *stackbase;
  size_t stksiz;
  unsigned int *stacktop;
  int i, threadId;
  _pthread_status_t *thread;
  int cogid;

  _lock_pthreads();
  for (i = 0; i < _NUM_PTHREADS; i++)
    {
      thread = &_PTHREAD[i];
      if (!(thread->flags & _PTHREAD_ALLOCATED))
	break;
    }
  threadId = i;
  if (threadId >= _NUM_PTHREADS)
    {
      _unlock_pthreads();
      errno = EAGAIN;
      return -1;
    }
  thread->flags = _PTHREAD_ALLOCATED;
  thread->TLSdata.thread_id = threadId+1;
  _unlock_pthreads();

  /* set up the stack */
  if (attr->stksiz)
    stksiz = attr->stksiz;
  else
    stksiz = 1024;

  /* stksiz must be at least 16 bytes or fail */
  if (stksiz < 16)
    {
      thread->flags = 0;
      errno = EINVAL;
      return -1;
    }
  if (attr->stack != NULL) {
    stackbase = attr->stack;
  } else {
    stackbase = malloc(stksiz);
    if (!stackbase) {
      thread->flags = 0;
      errno = ENOMEM;
      return -1;
    }
    thread->flags |= _PTHREAD_FREE_STACK;
  }

  thread->startfunc = startfunc;
  thread->arg = arg;

  /* push things onto the stack */
  stacktop = (unsigned int *)stackbase;
  stacktop += stksiz/4;

  /* run the function */
  cogid = _start_cog_thread(stacktop, pthread_run, thread, &thread->TLSdata);
  if (cogid < 0)
    {
      thread->flags = 0;
      errno = EAGAIN;
      return -1;
    }
  thread->cogid = cogid;
  *threadId_ptr = threadId;
  return 0;
}

/*
 * find our own thread id
 */
pthread_t
pthread_self(void)
{
  return _TLS->thread_id;
}

/*
 * terminate a thread
 */
void
pthread_exit(void *result)
{
  int id;
  _pthread_status_t *thread;
  id = pthread_self() - 1;

  /* pthread_exit on the main thread is the same as exit() */
  if (id < 0)
    exit(0);

  thread = &_PTHREAD[id];
  thread->arg = result;
  thread->flags |= _PTHREAD_TERMINATED;
  if ((thread->flags & _PTHREAD_FREE_STACK) && thread->stackbase)
    {
      free(thread->stackbase);
      thread->stackbase = 0;
    }
  __builtin_propeller_cogstop(__builtin_propeller_cogid());
}

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
