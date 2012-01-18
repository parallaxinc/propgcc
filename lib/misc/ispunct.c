#include <ctype.h>

#undef ispunct

int ispunct(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTp));
}
