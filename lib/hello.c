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

/*
 * NOTE: normally argv and environ are empty. We can change this
 * by adding global variables
 * char *_argv[] = { "arg0", "arg1", ..., NULL };
 * char *_environ[] = { "VAR1=val1", "VAR2=val2", ..., NULL };
 */

int
main(int argc, char **argv, char **environ)
{
  printf("hello, world\n");
  fprintf(stderr, "%ls\n", L"hello, stderr");
  return 99;
}
