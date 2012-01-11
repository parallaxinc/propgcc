#include <wctype.h>

int
iswctype(wint_t wc, wctype_t desc)
{
  int ok = 0;
  unsigned int mask;
  unsigned int cprop;

  /* for now our table only contains entries for the first 255 characters */
  if (wc < 0 || wc > 255)
    return 0;
  cprop = __ctype[wc];  /* character properties */

  mask = desc & _CTmask;

  if (desc & _CTanybut) {
    ok = (cprop  != 0) && ((mask & cprop) == 0);
  } else {
    ok = (cprop & mask) != 0;
  }
  if (desc & _CTnot)
    ok = !ok;
  return ok;
}
