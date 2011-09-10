/*
 * public domain memcpy
 * written by Eric Smith
 */
#include <string.h>

void *memcpy(void *dest_p, const void *src_p, size_t n)
{
  const char *src = (const char *)src_p;
  char *dest;

  while (n > 0) {
    *dest++ = *src++;
    --n;
  }
  return dest_p;
}
