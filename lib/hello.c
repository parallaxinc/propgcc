#include <stdio.h>
#include <sys/driver.h>
#include <compiler.h>

#ifndef SIMPLE
/* list of drivers we can use */
extern _Driver _FullDuplexSerialDriver;

_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  NULL
};
#endif

int
main(void)
{
  printf("hello, world\n");
  fprintf(stderr, "hello, stderr\n");
  return 99;
}
