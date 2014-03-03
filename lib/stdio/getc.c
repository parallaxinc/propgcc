/*
 * just in case someone does not include stdio.h
 */
#include <stdio.h>

#undef getc

int
getc(FILE *f)
{
  return fgetc(f);
}
