#include <ctype.h>

#undef isgraph

int isgraph(int c)
{
  return  (!(__ctype[(unsigned char)(c)]&(_CTc|_CTs)) && (__ctype[(unsigned char)(c)]));
}
