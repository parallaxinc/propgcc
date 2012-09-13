#include <stdlib.h>
#include <compiler.h>

lldiv_t lldiv(long long n, long long d)
{
  lldiv_t r;

  r.quot = n / d;
  r.rem = n % d;
  return r;
}

__weak_alias(imaxdiv, lldiv);
