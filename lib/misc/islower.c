#include <ctype.h>

#undef islower

int islower(int c)
{
  return __isctype(c, _CTl);
}
