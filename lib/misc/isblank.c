#include <ctype.h>

#undef isblank

int isblank(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTb));
}
