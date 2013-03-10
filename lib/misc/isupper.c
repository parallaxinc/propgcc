#include <ctype.h>

#undef isupper

int isupper(int c)
{
  return __isctype(c, _CTu);
}
