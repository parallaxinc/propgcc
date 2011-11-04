#define _BSD_SOURCE 1
#include <stdio.h>

void setlinebuf(FILE *fp)
{
  setvbuf(fp, NULL, _IOLBF, 0);
}
