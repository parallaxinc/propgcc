#include <ctype.h>

#undef isdigit

int isdigit(int c)
{
  return __isctype(c, _CTd);
}
