/*
 * POSIX function to open memory as a file
 * Written by Eric R. Smith and placed in the public domain
 */

#define _GNU_SOURCE
#include <stdio.h>

extern struct __driver _memory_driver;

FILE *fmemopen(void *buf, size_t size, const char *mode)
{
  FILE *fp;

  fp = __fopen_findslot(&_memory_driver);
  if (!fp) return fp;
  return __string_file(fp, buf, mode, size);
}
