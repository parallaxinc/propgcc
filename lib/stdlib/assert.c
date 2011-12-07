#include <stdio.h>
#include <assert.h>

void __eprintf(const char *condexpr, unsigned long line, const char *file)
{
  char buf[12];

  /* convert "line" to a string */
  /* this would all be easier with fprintf, of course, but we
     don't want to pull that in if we can avoid it (it's a big
     function! */

  char *ptr = &buf[11];
  *ptr = 0;
  do {
    --ptr;
    *ptr = (line % 10) + '0';
    line = line / 10;
  } while (line != 0);

  /* now print out the message */
  fputs("assertion ", stderr);
  fputs(condexpr, stderr);
  fputs(" failed at line ", stderr);
  fputs(ptr, stderr);
  fputs(" of file ", stderr);
  fputs(file, stderr);
  fputs("\n", stderr);
}
