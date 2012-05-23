/*
 * @fgetc.c
 * Implementation of stdio library functions
 * Originally from the public domain dLibs library via the MiNT library
 * Propeller specific changes
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
#include <stdio.h>

int _filbuf(FILE *fp)
{
    register unsigned int f;
    register long got;
    
    f = fp->_flag;
    if(f & _IORW) f = (fp->_flag |= _IOREAD);
    if(!(f & _IOREAD) || (f & (_IOERR | _IOEOF)))
	return(EOF);

    /* if this is stdin &  a tty, and stdout is line buffered, flush it */
    if((fp == stdin) && (f & _IODEV) && (stdout->_flag & _IOLBF))
	(void)fflush(stdout);

    fp->_ptr = fp->_base;
    if((got = fp->_drv->read(fp, fp->_base, (unsigned long)fp->_bsiz)) <= 0)
    {   /* EOF or error */
	fp->_flag |= ((got == 0) ? ((f & _IODEV) ? 0 : _IOEOF) : _IOERR);
	fp->_cnt = 0;
	return EOF;
    }
    fp->_cnt = got - 1;
    return *(fp->_ptr)++;
}

int
fgetc(FILE *fp)
{
  int c;

  c = --fp->_cnt >= 0 ? ((int)*fp->_ptr++) : _filbuf(fp);
  return c;
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
