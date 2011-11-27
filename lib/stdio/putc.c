/*
 * just in case someone does not include stdio.h
 */
#include <stdio.h>

#undef putc

int
putc(int c, FILE *f)
{
  return fputc(c, f);
}
