#include <stdlib.h>

int
mblen(const char *s, size_t n)
{
  if (s == NULL)
    return 0; /* no state dependent encodings */

  if (*s == 0 || n == 0)
    return 0;  /* empty string */

  return 1;
}

int
mbtowc(wchar_t *__restrict pwc, const char *__restrict s, size_t n)
{
  int c;
  if (s == NULL || pwc == NULL || n == 0)
    return mblen(s, n);
  c = *(unsigned char *)s;
  *pwc = c;
  return (c != 0);
}
