/**
 * @file include/compiler.h
 *
 * @brief Defines features of the compiler being used.
 *
 * @details
 * This file defines various features of the compiler being used such as:
 * @li _INT_SIZE   2 for 16 bit, 4 for 32 bit
 * @li _LONG_SIZE  4 for 32 bit, 8 for 64 bit
 * @li _WCHAR_SIZE sizeof(wchar_t)
 * @li _CHAR_IS_UNSIGNED 1 or 0, depending
 * @li _NORETURN: attribute indicating a function does not return
 * @li _CONST:    indicates that the function does not change or examine memory
 */

#ifndef _COMPILER_H
#define _COMPILER_H

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

#ifndef _WCHAR_T_TYPE
#define _WCHAR_T_TYPE __WCHAR_TYPE__
#endif
#define _CONSTRUCTOR __attribute__((constructor))
#define _DESTRUCTOR __attribute__((destructor))
#ifdef __PROPELLER_CMM__
#define _CACHED     __attribute__((fcache))
#else
#define _CACHED
#endif

#define _NAN __builtin_nan("1")
#define _NANL __builtin_nanl("1")
#define _NANF __builtin_nanf("1")
#ifndef __weak_alias
#define __weak_alias(sym, oldfunc) \
  __asm__( " .weak _" #sym "\n  .equ _" #sym ",_" #oldfunc "\n" )
#endif
#ifndef __strong_alias
#define __strong_alias(sym, oldfunc) \
  __asm__( " .global _" #sym "\n  .equ _" #sym ",_" #oldfunc "\n" )
#endif

#else

#error "compiler not yet supported"

#endif


#endif
