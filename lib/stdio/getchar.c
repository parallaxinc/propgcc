/*
 * just in case someone does not include stdio.h
 */
#include <stdio.h>

#undef getchar

int
getchar()
{
  return fgetc(stdin);
}
