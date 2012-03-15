#include <ctype.h>

#undef isalpha

int isalpha(int c)
{
  return __isctype(c, _CTalpha);
}
