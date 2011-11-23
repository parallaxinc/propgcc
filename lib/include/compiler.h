#ifndef _COMPILER_H
#define _COMPILER_H

/*
 * this file defines various features of the compiler being used
 * such as:
 * _INT_SIZE   2 for 16 bit, 4 for 32 bit
 * _LONG_SIZE  4 for 32 bit, 8 for 64 bit
 * _WCHAR_SIZE sizeof(wchar_t)
 * _CHAR_IS_UNSIGNED 1 or 0, depending
 *
 * _NORETURN: attribute indicating a function does not return
 * _CONST:    indicates that the function does not change or examine memory
 */

#if defined(__GNUC__)
#define _INT_SIZE  __SIZEOF_INT__
#define _LONG_SIZE __SIZEOF_LONG__
#define _WCHAR_SIZE __SIZEOF_WCHAR_T__
#ifdef __CHAR_UNSIGNED__
#define _CHAR_IS_UNSIGNED 1
#else
#define _CHAR_IS_UNSIGNED 0
#endif
#define _NORETURN __attribute__((noreturn))
#define _CONST    __attribute__((const))
#define _WEAK     __attribute__((weak))

#define _CONSTRUCTOR __attribute__((constructor))
#define _DESTRUCTOR __attribute__((destructor))

#define _NAN __builtin_nan("1")
#define _NANL __builtin_nanl("1")
#define _NANF __builtin_nanf("1")

#else

#error "compiler not yet supported"

#endif


#endif
