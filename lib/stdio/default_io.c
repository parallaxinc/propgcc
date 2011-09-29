/*
 * default _InitIO function to set up stdio, stderr, etc.
 */

#include <driver.h>
#include <compiler.h>

/* list of drivers we can use */
extern _Driver _SimpleSerialDriver;

_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  NULL
};

/* initialize I/O */
_CONSTRUCTOR void
_InitIO(void)
{
  /* open stdin */
  __fopen_driver(stdin, _driverlist[0], "", "r");
  /* open stdout */
  __fopen_driver(stdout, _driverlist[0], "", "w");
  /* open stderr */
  __fopen_driver(stderr, _driverlist[0], "", "w");

  /* default to flushing on every newline */
  stdout->_flag |= _IOLBF;
  stderr->_flag |= _IOLBF;
}

/* tear down I/O */
_DESTRUCTOR void
_FinishIO(void)
{
  extern void _serial_break(void);

  _serial_break();
}
