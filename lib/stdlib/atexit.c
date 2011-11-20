/*
 * @atexit.c
 * Register functions to be called at exit.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdlib.h>
#include <stdio.h>
#include <compiler.h>

struct _atexit_handler {
  struct _atexit_handler *next;
  void (*func)(void *);
  void *arg;
};

struct _atexit_handler *__atexitlist = 0;

/* the C++ runtime needs slightly more sophisticated atexit handling,
 * which is provided by __cxa_atexit instead of atexit
 */
int
__cxa_atexit(void (*function)(void *), void *arg, void *dso)
{
  struct _atexit_handler *ah;

  ah = malloc(sizeof(*ah));
  if (ah == NULL)
    {
      return -1;
    }
  ah->next = __atexitlist;
  ah->func = function;
  ah->arg = arg;
  __atexitlist = ah;
  return 0;
}

int
atexit(void (*func)(void))
{
  return __cxa_atexit((void (*)(void *))func, NULL, NULL);
}

/*
 * destructor to call all the registered functions
 */

_DESTRUCTOR
static void
_run_atexit(void)
{
  struct _atexit_handler *ah;

  /* run the atexit functions */
  for (ah = __atexitlist; ah; ah = ah->next)
    {
      (*ah->func)(ah->arg);
    }
}

__attribute__((weak)) char __dso_handle;

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
