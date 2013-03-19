/*
 * simple serial driver
 * Copyright (c) Parallax Inc. 2011
 * MIT Licensed (see end of file)
 */

/*
 * special hook which sends a sequence which propeller-load recognizes
 * as an exit
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cog.h>
#include <errno.h>
#include <sys/driver.h>

#define _serial_tx(x) (f->_drv->putbyte((x), f))
extern _Driver _SimpleSerialDriver;

__attribute__((section(".hubtext")))
void
_serial_exit(int n)
{
  FILE fbase;
  FILE *f = &fbase;

  memset(f, 0, sizeof(*f));
  __fopen_driver(f, &_SimpleSerialDriver, "", "w");

  _serial_tx(0xff);
  _serial_tx(0x00);
  _serial_tx(n & 0xff);
  __builtin_propeller_cogstop(__builtin_propeller_cogid());
}

#ifdef __GNUC__
void _ExitHook(int) __attribute__ ((alias ("_serial_exit")));
#else
void _ExitHook(int n)
{
  _serial_exit(n);
}
#endif
