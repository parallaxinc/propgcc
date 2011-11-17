#define _BSD_SOURCE 1
#include <stdio.h>

void setbuffer(FILE *fp, char *buf, size_t size)
{
  setvbuf(fp, buf, buf ? _IOFBF : _IONBF, size);
}
