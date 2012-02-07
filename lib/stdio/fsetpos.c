/* from Dale Schumacher's dLibs */

#include <stdio.h>

int
fsetpos(fp, pos)
  FILE *fp;
  fpos_t *pos;
{
  register long rv;

  if (pos)
  {
    rv = fseek(fp, *pos, SEEK_SET);
    if (rv >= 0)
    {
      fp->_flag &= ~(_IOEOF|_IOERR);
      return (0);
    }
  }
  return (-1);
}
