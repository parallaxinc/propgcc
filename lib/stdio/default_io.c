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
