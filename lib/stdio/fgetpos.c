/* from Dale Schumacher's dLibs */

#include <stdio.h>

int
fgetpos(fp, pos)
  FILE *fp;
  fpos_t *pos;
{
  register long rv;

  rv = ftell(fp);
  if ((rv >= 0) && pos)
  {
    *pos = rv;
    return (0);
  }
  return (-1);
}
