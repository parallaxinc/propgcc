#include <ctype.h>

#undef islower

int islower(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTl));
}
