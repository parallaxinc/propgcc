#include <ctype.h>

#undef isalnum

int isalnum(int c)
{
  return __isctype(c, _CTalnum);
}
