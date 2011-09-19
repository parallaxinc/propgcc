#include <stdlib.h>

/* dummy implementation of malloc; improve this when we need free()! */

void *
malloc(size_t n)
{
  extern char *_sbrk(unsigned long n);
  return (void *)_sbrk(n);
}
