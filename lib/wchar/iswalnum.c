#include <wctype.h>

int
iswalnum(wint_t wc)
{
  return iswctype(wc, _CTalnum);
}
