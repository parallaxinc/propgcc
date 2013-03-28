/*
 * @malloc.c
 * Implementation of memory allocation functions
 *
 * This is an extremely simple memory allocator, vaguely inspired
 * by the one in K&R.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */
typedef struct MemHeader {
  struct MemHeader *next;  /* pointer to next member, if on free list */
  size_t len;
} MemHeader;

typedef struct MemHeap {
    void *(*sbrk)(unsigned long n);
    MemHeader *free;
} MemHeap;

/* magic number to indicate allocated memory */
#define MAGIC ((MemHeader *)0xa110c)

/* normal and hub heaps */
extern MemHeap _malloc_heap;
extern MemHeap _hub_malloc_heap;

/* internal malloc function that can allocate from either heap */
void *_common_malloc(MemHeap *heap, size_t n);

/* function to request memory from the OS */
extern void *_sbrk(unsigned long n);
extern void *_hubsbrk(unsigned long n);

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
