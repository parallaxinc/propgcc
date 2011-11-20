#include <stdlib.h>
#include <compiler.h>

ldiv_t ldiv(long n, long d)
{
  ldiv_t r;

  r.quot = n / d;
  r.rem = n % d;
  return r;
}

#if (_INT_SIZE == _LONG_SIZE) && defined(__GNUC__)
div_t div(int n, int d) __attribute__((alias("ldiv")));
#else
div_t div(int n, int d)
{
  div_t r;

  r.quot = n / d;
  r.rem = n % d;
  return r;
}
#endif
