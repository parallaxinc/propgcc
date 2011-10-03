#ifndef _DRIVER_H
#define _DRIVER_H

#include <stdio.h>
#include <sys/driver.h>

#ifdef __GNUC__

#define INCLUDE_DRIVER(x) extern _Driver x; __asm__("\t.global\t_" #x);
void _InitIO(void) __attribute__((constructor));

#else

#define INCLUDE_DRIVER(x) extern _Driver x; _Driver *x ## _fp = &x;
void _InitIO(void);

#endif



#endif
