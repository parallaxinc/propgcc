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

/* the open file descriptors */
FILE __files[FOPEN_MAX];

/* dummy i/o functions */
static int
no_read(FILE *fp)
{
  return EOF;
}

static int
no_write(int c, FILE *fp)
{
  return EOF;
}

/*
 * special function to force all data to be appended
 */
static int
write_at_end(int c, FILE *fp)
{
  if (0 != (*fp->drv->seek)(fp, 0L, SEEK_END))
    return EOF;
  return (*fp->drv->putbyte)(c, fp);
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
   * find an open slot
   */
  for (i = 0; i < FOPEN_MAX; i++)
    {
      if (__files[i].drv == NULL)
	break;
    }
  if (i == FOPEN_MAX)
    {
      errno = EMFILE;
      return NULL;
    }

  fp = &__files[i];

  /*
   * find the driver corresponding to the name
   * every driver has a prefix, like "DOS:"
   */

  for (i = 0; (d = _driverlist[i]) != 0; i++)
    {
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

  fp->flags = 0;
  fp->putbyte = no_write;
  fp->getbyte = no_read;

  if (mode[0] == 'r') {
    fp->flags |= _SFINP;
    if (mode[1] == '+')
      fp->flags |= _SFOUT;
  }
  if (mode[0] == 'w' || mode[0] == 'a')
    {
      fp->flags |= _SFOUT;
      if (mode[1] == '+')
	fp->flags |= _SFINP;
      if (mode[0] == 'a')
	{
	  fp->flags |= _SFAPPEND;
	}
    }

  if (fp->flags & _SFINP)
    fp->getbyte = d->getbyte;
  if (fp->flags & _SFOUT)
    {
      if (d->seek && (fp->flags & _SFAPPEND))
	fp->putbyte = write_at_end;
      else
	fp->putbyte = d->putbyte;
    }

  /* see if the driver is happy about this file */
  i = (*d->fopen)(fp, name + plen, mode);
  if (i < 0)
    {
      /* driver unhappy, it should have set errno */
      return NULL;
    }

  fp->drv = d;
  return fp;
}

/*
 * close a file
 */
int
fclose(FILE *fp)
{
  if (fp->drv != 0)
    {
      errno = EBADF;
      return EOF;
    }
  fflush(fp);
  (*fp->drv->fclose)(fp);
  fp->drv = 0;
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
