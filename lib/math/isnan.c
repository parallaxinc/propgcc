#include <math.h>
#include "math_private.h"

int
__isnan(double x)
{
  return __builtin_isnan(x);
}

int
__isnanl(long double x)
{
  return __builtin_isnanl(x);
}

int
__isnanf(float x)
{
  return __builtin_isnanf(x);
}
