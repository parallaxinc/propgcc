/**
 * @file include/driver.h
 *
 * @brief Provides driver API functions for initializing devices.
 *
 * Copyright (c) 2011-2012 by Parallax, Inc.
 * All rights MIT Licensed.
 *
 * The _InitIO() function is called at program startup before main().
 *
 * It initializes stdio devices as defined in include/sys/driver.h
 *
 * New devices having stdio interfaces can be added to the driver table.
 */
#ifndef _DRIVER_H
#define _DRIVER_H

#include <stdio.h>
#include <sys/driver.h>

#ifdef __GNUC__

#define INCLUDE_DRIVER(x) extern _Driver x; __asm__("\t.global\t_" #x);
/** @see Details above */
void _InitIO(void) __attribute__((constructor));

#else

#define INCLUDE_DRIVER(x) extern _Driver x; _Driver *x ## _fp = &x;
/** @see Details above */
void _InitIO(void);

#endif



#endif
