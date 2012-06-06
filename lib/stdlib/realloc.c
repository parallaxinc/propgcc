/*
 * @realloc.c
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
#include <sys/thread.h>
#include "malloc.h"

/* local functions */
static void *common_realloc(MemHeap *heap, void *ptr, size_t n);

/*
 * realloc: expand or shrink a block
 * ideally when expanding we would look for a free block just after this one and
 * expand it on request, but for now we just punt
 * the shrinking we really do correctly though
 */
void *
realloc(void *ptr, size_t n)
{
    return common_realloc(&_malloc_heap, ptr, n);
}

void *
hubrealloc(void *ptr, size_t n)
{
#if defined(__PROPELLER_LMM__) || defined(__PROPELLER_XMMC__)
    return common_realloc(&_malloc_heap, ptr, n);
#else
    return common_realloc(&_hub_malloc_heap, ptr, n);
#endif
}

static void *
common_realloc(MemHeap *heap, void *ptr, size_t n)
{
  MemHeader *thisp, *newp;
  void *newptr;
  size_t numunits;

  if (ptr == NULL)
    return malloc(n);
  if (n == 0)
    {
      free(ptr);
      return NULL;
    }

  thisp = (MemHeader *)ptr;
  --thisp;
  if (thisp->next != MAGIC)
    {
      return NULL; /* cannot do anything */
    }

  numunits = (n + sizeof(MemHeader)-1)/sizeof(MemHeader) + 1;
  if (thisp->len == numunits)
    {
      /* nothing to do, just return it unchanged */
      return ptr;
    }
  if (thisp->len > numunits)
    {
      /* shrink the block */
      newp = thisp + numunits;
      newp->next = MAGIC;
      newp->len = thisp->len - numunits;
      thisp->len -= numunits;
      free(newp+1);
      return ptr;
    }

  /* OK, the tricky case: allocate a new block and copy the memory into it */
  newptr = _common_malloc(heap, n);
  if (!newptr)
    return NULL;
  memcpy(newptr, ptr, (thisp->len-1)*sizeof(MemHeader));
  free(ptr);
  return newptr;
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
