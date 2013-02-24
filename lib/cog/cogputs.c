#include <stdio.h>

int
puts(const char *str)
{
  int c, r;
  r = 0;
  while (0 != (c = *str++))
    putchar(c);
  putchar('\n');
  return 0;
}
