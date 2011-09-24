/*
 * @exit.c
 * The last function called.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/exit.h>

struct _atexit_handler *__atexitlist = 0;

void
exit(int status)
{
  int i;
  struct _atexit_handler *ah;

  /* run the atexit functions */
  for (ah = __atexitlist; ah; ah = ah->next)
    {
      (*ah->func)();
    }

  /* clean up the buffers */
  for (i = FOPEN_MAX-1; i >= 0; --i)
    {
      fclose(&__file[i]);
    }

  /* now really exit */
  _Exit(status);
}
