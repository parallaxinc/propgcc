#include <ctype.h>

#undef isalpha

int isalpha(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTu|_CTl));
}
