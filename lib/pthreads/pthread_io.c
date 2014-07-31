/*
 * default _InitIO function to set up stdio, stderr, etc.
 * This differs from the one in the "normal" library because
 * SimpleSerialDriver is not well suited to multiply threaded
 * programs (it may not work right if called on different cogs)
 * FullDuplexSerialDriver does not have this issue.
 */

#include <driver.h>
#include <compiler.h>

/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;

_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  NULL
};
