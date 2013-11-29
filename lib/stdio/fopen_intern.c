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
#include <propeller.h>

/* lock for stdio functions */
HUBDATA atomic_t __stdio_lock;

/* force _InitIO to be linked */
extern void _InitIO(void);
long __dummy = (long)&_InitIO;

/* the open file descriptors */
/* these must go in HUB RAM so they can be shared amongst threads */
__attribute__((section(".hubdata"))) FILE __files[FOPEN_MAX];

/*
 * the fopen worker routine
 * this one takes a FILE * and sets it up for I/O
 * according to "mode", using the driver "d"
 */
FILE *
__fopen_driver(FILE *fp, _Driver *d, const char *name, const char *mode)
{
  int i;
  int flag = 0;

  /* force a reference to _driverlist */
  if (_driverlist[0] == 0)
    return NULL;

  flag = 0;

  if (mode[0] == 'r')
    {
      flag = _IOREAD;
      if (mode[1] == '+')
	flag = _IORW;
    }
  else if (mode[0] == 'w' || mode[0] == 'a')
    {
      flag = _IOWRT;
      if (mode[1] == '+')
	flag = _IORW;
      if (mode[0] == 'a')
	{
	  flag |= _IOAPPEND;
	}
    }

  fp->_flag = flag;
  fp->_base = NULL;  /* assume null buffer */

  /* see if the driver is happy about this file */
  if (d->fopen)
    {
      i = (*d->fopen)(fp, name, mode);
      if (i < 0)
	{
	  /* driver unhappy, it should have set errno */
	  fp->_base = NULL;
	  fp->_ptr = NULL;
	  fp->_bsiz = 0;
	  fp->_flag = 0;
	  fp->_drv = 0;
	  return NULL;
	}
    }

  /* driver may have allocated a buffer; if not, use a small one */
  if (NULL == fp->_base)
    {
      fp->_base = &fp->_chbuf[0];
      fp->_bsiz = _SMALL_BUFSIZ;
      /* this is probably a terminal, so apply line buffering */
      fp->_flag |= _IOLBF;
    }
  fp->_ptr = fp->_base;
  fp->_cnt = 0;
  fp->_drv = d;
  if (flag & _IOAPPEND) {
    if (d->seek)
      (*d->seek)(fp, 0L, SEEK_END);
  }
  return fp;
}

/*
 * close a file
 */
int
fclose(FILE *fp)
{
  int error = 0;

  if (fp == NULL || fp->_drv == NULL || 0 == (fp->_flag & (_IORW|_IOREAD|_IOWRT)))
    {
      errno = EBADF;
      return EOF;
    }
  if (fp->_flag & _IOWRT)
    fflush(fp);
  if (fp->_drv->fclose)
    error |= (*fp->_drv->fclose)(fp);
  fp->_base = NULL;
  fp->_ptr = NULL;
  fp->_bsiz = 0;
  fp->_flag = 0;
  fp->_drv = 0;
  return error ? EOF : 0;
}

/*
 * arrange to close files automatically at end of program
 */

_DESTRUCTOR static void
_do_stdio_cleanup(void)
{
  int i;

  fflush(NULL);  /* flush all data out */

  /* close all buffers */
  for (i = 0; i < FOPEN_MAX; i++)
    {
      fclose(&__files[i]);
    }

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
