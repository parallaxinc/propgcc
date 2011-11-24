#include <ctype.h>

#undef isalnum

int isalnum(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTu|_CTl|_CTd));
}
