#include <stdio.h>
#include <sys/driver.h>
#include <compiler.h>

/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;

_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  NULL
};
/* initialize I/O */
_CONSTRUCTOR void
_InitIO(void)
{
  /* open stdin */
  __fopen_driver(stdin, _driverlist[0], "", "r+");
  /* copy stdin to stdout and stderr */
  *stdout = *stdin;
  *stderr = *stdout;
}

int
main(void)
{
  printf("hello, world\n");
  return 0;
}
