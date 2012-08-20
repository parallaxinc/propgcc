#include <stdio.h>
#include <unistd.h>

int unlink(const char *name)
{
  return remove(name);
}
