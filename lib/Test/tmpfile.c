#include <stdio.h>

int
main()
{
  char buf[L_tmpnam];

  printf("tmpnam 0: %s\n", tmpnam(buf));
  printf("tmpnam 1: %s\n", tmpnam(buf));
  return 0;
}
