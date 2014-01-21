/*
 * @fopen.c
 * Implementation of stdio library functions
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <compiler.h>

/*
 * find an empty slot
 */
FILE *
__fopen_findslot(_Driver *d)
{
  int i;
  FILE *fp = NULL;

  __lock_stdio();
  /*
   * find an open slot
   */
  for (i = 0; i < FOPEN_MAX; i++)
    {
      if (__files[i]._drv == NULL)
	break;
    }
  if (i == FOPEN_MAX)
    {
      errno = EMFILE;
      __unlock_stdio();
      return NULL;
    }

  fp = &__files[i];
  fp->_drv = d;  /* mark it used */
  __unlock_stdio();
  return fp;
}

/*
 * the actual fopen routine
 */
FILE *
fopen(const char *name, const char *mode)
{
  size_t plen = 0;
  _Driver *d;
  int i;
  FILE *fp;

  /*
   * find the driver corresponding to the name
   * every driver has a prefix, like "DOS:"
   */

  for (i = 0; (d = _driverlist[i]) != 0; i++)
    {
      if (!d->prefix)
	continue;
      plen = strlen(d->prefix);
      if (!strncmp(d->prefix, name, plen))
	break;
    }

  if (!d)
    {
      /* driver not found */
      errno = ENOENT;
      return NULL;
    }
  fp = __fopen_findslot(d);
  if (!fp) return fp;

  return __fopen_driver(fp, d, name+plen, mode);
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
