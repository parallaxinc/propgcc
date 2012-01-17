/*
 * just in case someone does not include stdio.h
 */
#include <stdio.h>

#undef putchar

int
putchar(int c)
{
  return fputc(c, stdout);
}
