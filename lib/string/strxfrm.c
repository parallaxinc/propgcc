/* from the public domain libnix library */

#include <string.h>

size_t
strxfrm (char *to, const char *from, size_t maxsize)
{
  strncpy (to, from, maxsize);
  return strlen (from);
}
