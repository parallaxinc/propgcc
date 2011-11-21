#include <ctype.h>

#undef iscntrl

int iscntrl(int c)
{
  return (__ctype[(unsigned char)(c)]&(_CTc));
}
