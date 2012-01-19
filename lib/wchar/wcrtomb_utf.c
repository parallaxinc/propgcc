#include <wchar.h>
#include <stdint.h>
#include <errno.h>

size_t
_wcrtomb_utf8(char *s, wchar_t wcorig, mbstate_t *ps)
{
  uint32_t wc = wcorig;
  uint32_t count;
  uint32_t left;
  uint32_t init;

  if (wc <= 0x7f) {
    init = wc;
    wc = 0;
    count = 1;
  } else if (wc <= 0x7ff) {
    /* 10 bits valid */
    init = 0xC0;
    wc = (wc << (22-1));
    count = 2;
  } else if (wc <= 0xffff) {
    /* 16 bits valid */
    init = 0xE0;
    wc = (wc << (16-2));
    count = 3;
  } else if (wc <= 0x10ffff) {
    /* 22 bits (perhaps) valid */
    init = 0xF0;
    wc = (wc << (10-3));
    count = 4;
  } else {
    errno = EILSEQ;
    return (size_t)-1;
  }

  left = count;
  do {
    if (s) *s++ = (init | ((wc >> 24) & 0x3F));
    wc = wc << 6;
    init = 0x80;
    --left;
  } while (left > 0);

  return count;
}
