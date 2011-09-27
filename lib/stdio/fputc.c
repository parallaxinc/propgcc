/*
 * @fputc.c
 * Implementation of stdio library functions
 * Original from dLibs library, via the MiNT library
 * Propeller specific modifications are
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
#include <stdio.h>

int
fputc(int c, FILE *fp)
{
	register long m;
	unsigned int f = fp->_flag;

	if(f & _IORW)
	{
	    fp->_flag |= _IOWRT;
	    f = (fp->_flag &= ~(_IOREAD | _IOEOF));
	}
	if(!(f & _IOWRT)			/* not opened for write? */
	   || (f & (_IOERR | _IOEOF)))		/* error/eof conditions? */
		return(EOF);

	*(fp->_ptr)++ = c; fp->_cnt++;
	if( (fp->_cnt >= fp->_bsiz) || 
	    ((f & _IOLBF) && (c == '\n'))  ) /* flush line buffd stream on \n */
	{
		m = fp->_cnt;
		fp->_cnt = 0;
		fp->_ptr = fp->_base;
		if(fp->_drv->write(fp, fp->_base, m) != m) {
			fp->_flag |= _IOERR;
			return(EOF);
		}
	}
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
