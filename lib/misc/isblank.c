#include <ctype.h>

#undef isblank

int isblank(int c)
{
  return __isctype(c, _CTb);
}
