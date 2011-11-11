/*
 * null device driver (like /dev/null)
 *
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

#include <stdio.h>
#include <time.h>
#include <cog.h>
#include <sys/driver.h>
#include <errno.h>


int
_null_write(FILE *fp, unsigned char *buf, int size)
{
  return size;
}

int
_null_read(FILE *fp, unsigned char *buf, int size)
{
  return 0;
}

int
_null_fopen(FILE *fp, const char *str, const char *mode)
{
  return 0;
}


const char _NullPrefix[] = "NUL:";

_Driver _NullDriver =
  {
    _NullPrefix,
    _null_fopen,
    NULL,
    _null_read,
    _null_write,
    NULL,
    NULL,
    NULL,
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
