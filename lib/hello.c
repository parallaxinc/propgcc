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
  /* open the standard file handles */
  /* we could use freopen here, but __fopen_driver is a bit faster
     and drags in less library */
  __fopen_driver(stdin, _driverlist[0], "", "r");
  __fopen_driver(stdout, _driverlist[0], "", "w");
  __fopen_driver(stderr, _driverlist[0], "", "w");

  /* default to flushing on every newline */
  stdout->_flag |= _IOLBF;
  stderr->_flag |= _IOLBF;
}

int
main(void)
{
  printf("hello, world\n");
  fprintf(stderr, "hello, stderr\n");
  return 0;
}
