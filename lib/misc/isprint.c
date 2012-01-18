#include <ctype.h>

#undef isprint

int isprint(int c)
{
  return  (!(__ctype[(unsigned char)(c)]&_CTc) && (__ctype[(unsigned char)(c)]));
}
