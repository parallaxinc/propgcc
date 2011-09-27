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
  /* open stdin; we open it for writing too so that
     we can share with stdout and stderr (see below)
   */
  __fopen_driver(stdin, _driverlist[0], "", "r+");
  /* copy stdin to stdout and stderr */
  /* we do it this way because we want just one serial cog to be running
     if we opened new file handles for each one we would get new cogs
  */
  *stdout = *stdin;
  *stderr = *stdout;
}

int
main(void)
{
  printf("hello, world\n");
  return 0;
}
