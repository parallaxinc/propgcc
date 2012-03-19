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
#include <string.h>

int
pthread_attr_init(pthread_attr_t *attr)
{
  memset(attr, 0, sizeof(*attr));
  return 0;
}

/* nothing needs to be done for pthread_attr_destroy, so it
   can just re-initialize the attr structure
*/
#if defined(__GNUC__)
int pthread_attr_destroy(pthread_attr_t *attr) __attribute__((alias("pthread_attr_init")));
#else
int pthread_attr_destroy(pthread_attr_t *attr) 
{
  return pthread_attr_init(attr);
}
#endif

int
pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
  attr->stksiz = stacksize;
  return 0;
}
int
pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize)
{
  if (attr->stksiz == 0)
    *stacksize = _PTHREAD_STACK_DEFAULT;
  else
    *stacksize = attr->stksiz;
  return 0;
}

int
pthread_attr_getstackaddr(pthread_attr_t *attr, void **addr)
{
  *addr = attr->stack;
  return 0;
}
int
pthread_attr_setstackaddr(pthread_attr_t *attr, void *addr)
{
  attr->stack = addr;
  return 0;
}

int
pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate)
{
  *detachstate = (attr->flags & _PTHREAD_DETACHED) != 0;
  return 0;
}

int
pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate)
    attr->flags |= _PTHREAD_DETACHED;
  else
    attr->flags &= ~_PTHREAD_DETACHED;
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
