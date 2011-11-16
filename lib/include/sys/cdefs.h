#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H

#if defined(__GNUC__)
#ifndef __weak_alias
#define __weak_alias(sym, oldfunc) \
  __asm__( " .weak " #sym "\n  .equ " #sym "," #oldfunc "\n" )
#endif
#ifndef __strong_alias
#define __strong_alias(sym, oldfunc) \
  __asm__( " .global " #sym "\n  .equ " #sym "," #oldfunc "\n" )
#endif
#else
#error "must define __weak_alias and __strong_alias"
#endif

#endif
