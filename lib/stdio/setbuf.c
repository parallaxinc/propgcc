#include <stdio.h>

void setbuf(FILE *fp, char *buf)
{
  setvbuf(fp, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

