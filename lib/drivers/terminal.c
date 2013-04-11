/*
 * terminal driver for serial port I/O
 *
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

#include <stdio.h>
#include <cog.h>

__attribute__((section(".hubtext")))
int
_term_write(FILE *fp, unsigned char *buf, int size)
{
  int count = 0;
  int c;
  int (*putbyte)(int, FILE *);

  putbyte = fp->_drv->putbyte;
  if (!putbyte)
    return 0;
  while (count < size)
    {
      c = *buf++;
      if (c == '\n')
	putbyte('\r', fp);
      putbyte(c, fp);
      count++;
    }
  return count;
}

__attribute__((section(".hubtext")))
int
_term_read(FILE *fp, unsigned char *buf, int size)
{
  int (*putbyte)(int c, FILE *fp);
  int (*getbyte)(FILE *fp);
  int value;
  int count = 0;
  int cooked = fp->_flag & _IOCOOKED;
  int unbuffered = fp->_flag & _IONBF;

  putbyte = fp->_drv->putbyte;
  getbyte = fp->_drv->getbyte;
  if (!putbyte || !getbyte)
    return 0;

  if (!cooked)
    size = 1;

  while (count < size)
    {
      value = (*getbyte)(fp);
      if (value == -1)
	break;  /* EOF */

      /* convert cr to lf */
      if (cooked && value == '\r') {
	putbyte(value, fp); /* echo CR+LF */
	value = '\n';
      }
      buf[count++] = value;
      /* do cooked mode processing */
      /* for now this echos the input */
      if (cooked) {
	/* process backspace */
	if ( (value == '\b' || value == 0x7f) && !unbuffered)
	  {
	    if (count >= 2)
	      {
		count -= 2;
		putbyte('\b', fp);
		putbyte(' ', fp);
		putbyte('\b', fp);
	      }
	    else
	      {
		count = 0;
	      }
	  }
	else
	  putbyte(value, fp);
      }
      /* end of line stops the read */
      if (value == '\n') break;
    }
  return count;
}

/*
+--------------------------------------------------------------------
¦  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
