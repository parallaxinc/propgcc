/*
 * default _InitIO function to set up stdio, stderr, etc.
 */

#include <driver.h>

INCLUDE_DRIVER(_SimpleSerial)

void
_InitIO(void)
{
  /* open stdin */
  fopen("SSER:", "r");
  /* open stdout */
  fopen("SSER:", "w");
  /* open stderr */
  fopen("SSER:", "w");
}

extern _Driver _SimpleSerialDriver;

_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  NULL
};
