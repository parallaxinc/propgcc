/*
 * @fflush.c
 * Implementation of stdio library functions
 *
 * Originally from Dale Schumacher's dLibs
 * Propeller modifications:
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
#include <stdio.h>
#include <errno.h>
#include <sys/thread.h>

static int
_fflush(FILE *fp)
{
  register int f, rv = 0;
  register long offset;

  if(fp == NULL)
    return(0);
  f = fp->_flag;
  if (!(f & (_IORW | _IOREAD | _IOWRT)))  /* file not open */
    return(EOF);

  __lock(&fp->_lock);
  if(fp->_cnt > 0)	 		/* data in the buffer */
    {
      if(f & _IOWRT)				/* writing */
	{
	  register long	todo;

	  /* _cnt is cleared before writing to avoid */
	  /* loop if fflush is recursively called by */
	  /* exit if ^C is pressed during this write */
	  todo = fp->_cnt;
	  fp->_cnt = 0;
	  if(fp->_drv->write(fp, fp->_base, todo) != todo) 
	    {
	      fp->_flag |= _IOERR;
	      rv = EOF;
	    }
	}
      else if(f & _IOREAD) 			/* reading */
	{
	  offset = -(fp->_cnt);
	  if(fp->_drv->seek(fp, offset, SEEK_CUR) < 0)
	    if(!(f & _IODEV))
	      rv = EOF;
	}
    }
  if(f & _IORW)
    fp->_flag &= ~(_IOREAD | _IOWRT);
  fp->_ptr = fp->_base;
  fp->_cnt = 0;
  __unlock(&fp->_lock);
  return(rv);
}

int
fflush(FILE *fp)
{
  int i;
  if (fp)
    {
      return _fflush(fp);
    }
  /* fflush(NULL) means to flush all buffers */
  for (i = 0; i < FOPEN_MAX; i++)
    _fflush(&__files[i]);
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
