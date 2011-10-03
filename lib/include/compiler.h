#ifndef _COMPILER_H
#define _COMPILER_H

/*
 * this file defines various features of the compiler being used
 * such as:
 * _INT_SIZE   4 for 32 bit, 8 for 64 bit
 * _LONG_SIZE  4 for 32 bit, 8 for 64 bit
 * _WCHAR_SIZE sizeof(wchar_t)
 * _CHAR_IS_UNSIGNED 1 or 0, depending
 *
 * _NORETURN: attribute indicating a function does not return
 * _CONST:    indicates that the function does not change or examine memory
 */

#if defined(__propeller__) && defined(__GNUC__)
#define _LONG_SIZE 4
#define _LONG_SIZE 4
#define _CHAR_IS_UNSIGNED 1
#define _WCHAR_SIZE 4
#define _NORETURN __attribute__((noreturn))
#define _CONST    __attribute__((const))

#define _CONSTRUCTOR __attribute__((constructor))
#define _DESTRUCTOR __attribute__((destructor))
#else
#error "compiler not yet supported"
#endif

#endif
