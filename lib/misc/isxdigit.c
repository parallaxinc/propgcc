#include <ctype.h>

#undef isxdigit

int isxdigit(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTd|_CTx));
}
