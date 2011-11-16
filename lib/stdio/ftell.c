/* @ftell.c
 * Originally from the public domain dLibs library via the MiNT library
 * Propeller specific changes
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdio.h>
#include <errno.h>

long
ftell(FILE *fp)
{
  long rv, count = fp->_cnt, adjust = 0;
  unsigned int f = fp->_flag;

  if (f & _IOREAD)
    adjust = -count;
  else if (f & (_IOWRT | _IORW))
    {
      if(f & _IOWRT)
        adjust = count;
    }
  else return -1L;

  if (fp->_drv->seek)
    {
      rv = fp->_drv->seek(fp, 0L, SEEK_CUR);
    }
  else
    {
      rv = -1L;
      errno = ENOSEEK;
    }
  return (rv < 0) ? -1L : rv + adjust;
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
