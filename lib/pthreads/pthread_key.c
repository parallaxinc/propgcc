/*
 * pthread thread specific data implementation
 * Copyright (c) 2012 Parallax, Inc.
 * MIT licensed (see terms at end of file)
 */
#include <pthread.h>
#include <errno.h>

static unsigned char keyused[PTHREAD_KEYS_MAX];

int
pthread_key_create(pthread_key_t *key, __pthread_destruct_func destructor)
{
  int i;
  __lock_pthreads();
  for (i = 0; i < PTHREAD_KEYS_MAX; i++)
    {
      if (keyused[i] == 0) {
	keyused[i] = 1;
	__pdestruct[i] = destructor;
	break;
      }
    }
  __unlock_pthreads();
  if (i == PTHREAD_KEYS_MAX) {
    return EAGAIN;
  }
  *key = i;
  return 0;
}

int
pthread_key_delete(pthread_key_t key)
{
  int i = key;

  if (i < 0 || i >= PTHREAD_KEYS_MAX)
    return EINVAL;
  __lock_pthreads();
  if (!keyused[i]) {
    __unlock_pthreads();
    return EINVAL;
  }
  keyused[i] = 0;
  __pdestruct[i] = NULL;
  __unlock_pthreads();
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
