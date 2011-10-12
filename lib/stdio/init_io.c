/*
 * default _InitIO function to set up stdio, stderr, etc.
 */

#include <driver.h>
#include <compiler.h>
#include <sys/thread.h>

/* initialize I/O */
_CONSTRUCTOR void
_InitIO(void)
{
  /* open stdin */
  __fopen_driver(stdin, _driverlist[0], "", "r");
  /* make it "cooked" input, and give it a decent sized buffer */
  stdin->_flag |= _IOCOOKED;
  stdin->_base = stdin->_ptr = _TLS->linebuf;
  stdin->_bsiz = sizeof(_TLS->linebuf);

  /* open stdout */
  __fopen_driver(stdout, _driverlist[0], "", "w");
  /* open stderr */
  __fopen_driver(stderr, _driverlist[0], "", "w");
}
