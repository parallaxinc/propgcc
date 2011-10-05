/*
 * default _InitIO function to set up stdio, stderr, etc.
 */

#include <driver.h>
#include <compiler.h>

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
}
