/*
 * @calloc.c
 * Implementation of memory allocation functions
 *
 * This is an extremely simple memory allocator, vaguely inspired
 * by the one in K&R.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdlib.h>
#include <string.h>

void *
calloc(size_t nmemb, size_t len)
{
  void *ptr;
  len = nmemb*len;
  ptr = malloc(len);
  if (ptr)
    {
      memset(ptr, 0, len);
    }
  return ptr;
}

#if defined(__PROPELLER_LMM__) || defined(__PROPELLER_XMMC__)
/* in these modes all data is in hub, so hubcalloc is the same as calloc */
__strong_alias(_hubcalloc, calloc);
__weak_alias(hubcalloc, calloc);

#else

void *
_hubcalloc(size_t nmemb, size_t len)
{
  void *ptr;
  len = nmemb*len;
  ptr = _hubmalloc(len);
  if (ptr)
    {
      memset(ptr, 0, len);
    }
  return ptr;
}

/* provide a more convenient, but non-ANSI compliant, name */
__weak_alias(hubcalloc, _hubcalloc);

#endif


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
