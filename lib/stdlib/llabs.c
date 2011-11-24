#include <stdlib.h>
#include <compiler.h>

long long
llabs(long long n)
{
  return (n < 0) ? -n : n;
}

__weak_alias(imaxabs,llabs);
