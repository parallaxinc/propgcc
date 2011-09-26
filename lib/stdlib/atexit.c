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
  void (*func)(void);
};

struct _atexit_handler *__atexitlist = 0;

int
atexit(void (*function)(void))
{
  struct _atexit_handler *ah;

  ah = malloc(sizeof(*ah));
  if (ah == NULL)
    {
      return -1;
    }
  ah->next = __atexitlist;
  ah->func = function;
  __atexitlist = ah;
  return 0;
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
      (*ah->func)();
    }
}
