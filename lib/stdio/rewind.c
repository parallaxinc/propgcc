#include <stdio.h>

void rewind(FILE *stream)
{
  fseek(stream, 0L, SEEK_SET);
}
