#include <ctype.h>

#undef isspace

int isspace(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTs));
}
