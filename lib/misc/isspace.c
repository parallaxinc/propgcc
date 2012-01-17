#include <ctype.h>

#undef isspace

int isspace(int c)
{
  return __isctype(c, _CTs);
}
