#include <stdio.h>
#include <propeller.h>

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */

extern _Driver TvDriver;
extern _Driver _FileDriver;

_Driver *_driverlist[] = {
    &TvDriver,
    &_FileDriver,
    NULL
};
