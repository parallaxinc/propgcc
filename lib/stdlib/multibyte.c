#include <stdlib.h>
#include <wchar.h>

int
mbtowc(wchar_t *__restrict pwc, const char *__restrict s, size_t n)
{
  size_t x;
  mbstate_t mb = { 0, 0, 0 };

  x = _mbrtowc_ptr( pwc, s, n, &mb);
  if ( ((int)x) < 0 )
    return -1;
  return x;
}

int
mblen(const char *s, size_t n)
{
  return mbtowc(NULL, s, n);
}
