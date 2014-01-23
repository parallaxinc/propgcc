/*
 * pthread condition variables implementation
 * Copyright (c) 2012 Parallax, Inc.
 * MIT licensed (see terms at end of file)
 */
#include <pthread.h>
#include <errno.h>

extern _pthread_queue_t __ready_queue;

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
  cond->queue = 0;
  return 0;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
  return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
  if (!mutex->cnt)
    return EINVAL;  /* mutex should have been locked */
  __lock_pthreads();
  /* release the mutex */
  if (__addlock(&mutex->cnt, -1) != 0)
    {
      while (!_pthread_wake(&mutex->queue))
	{
	  _pthread_sleep_with_lock(&__ready_queue);
	}
    }
  /* wait for the condition to be signalled */
  _pthread_sleep_with_lock(&cond->queue);
  /* now re-aquire the mutex */
  if (__addlock(&mutex->cnt, 1) != 1) {
    do {
      _pthread_sleep_with_lock(&__ready_queue);
    } while (0 != pthread_mutex_trylock(mutex));
  }
  __unlock_pthreads();
  return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
  _pthread_wake(&cond->queue);
  return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
  _pthread_wakeall(&cond->queue);
  return 0;
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
