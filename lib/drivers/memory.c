/*
 * driver for reading/writing memory
 * this is used so that string I/O
 * (like sprintf, sscanf) can use the
 * same underlying routines as file I/O
 *
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

#include <stdio.h>
#include <time.h>
#include <cog.h>
#include <sys/driver.h>
#include <errno.h>

/*
 * FILE variable usage:
 * drvarg[0] == base memory address
 * drvarg[1] == max memory address
 * drvarg[2] == current memory position
 */

static int
mem_putc(int c, FILE *fp)
{
  char *base = (char *)fp->drvarg[0];
  long max = fp->drvarg[1];
  long cur = fp->drvarg[2];

  if (cur < max)
    {
      base[cur++] = c;
      fp->drvarg[2] = cur;
      return c;
    }
  else
    return EOF;
}

static int
mem_getc(FILE *fp)
{
  char *base = (char *)fp->drvarg[0];
  long max = fp->drvarg[1];
  long cur = fp->drvarg[2];
  int c;

  if (cur < max)
    {
      c = base[cur++];
      fp->drvarg[2] = cur;
      return c;
    }
  else
    return EOF;
}

static int
mem_seek(FILE *fp, long offset, int whence)
{
  long max = fp->drvarg[1];
  long cur = fp->drvarg[2];
  long pos;

  switch (whence)
    {
    case SEEK_SET:
      pos = offset;
      break;
    case SEEK_CUR:
      pos = cur + offset;
      break;
    case SEEK_END:
      pos = max + offset;
      break;
    default:
      errno = EINVAL;
      return -1;
    }
  if (pos < 0 || pos > max)
    {
      errno = EINVAL;
      return -1;
    }
  fp->drvarg[2] = pos;
  return 0;
}

static int
mem_fopen(FILE *fp, const char *str, const char *mode)
{
  fp->drvarg[0] = (unsigned long)str;
  fp->drvarg[1] = 0x7FFFFFFFL;  /* assume unlimited string, caller has to adjust this if necessary */
  fp->drvarg[2] = 0;
  return 0;
}

/*
 * force a terminating 0, if necessary
 */
static int
mem_fclose(FILE *fp)
{
  fp->putbyte(0, fp);
}

_Driver _memory_driver =
  {
    NULL,  /* no opening via normal fopen() */
    mem_fopen,
    NULL,  /* fclose hook, not needed */
    NULL,  /* flush hook, not needed */
    mem_getc,
    mem_putc,
    mem_seek,
    NULL
  };

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
