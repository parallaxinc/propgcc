#ifndef _SYS_THREAD_H
#define _SYS_THREAD_H

/*
 * thread local storage
 * the library should not keep anything in global or
 * static variables; all such values should be placed
 * in the TLS structure
 */
struct _TLS {
  int errno;
  char *strtok_scanpoint;
};

/*
 * we may want to define thread local storage in some special way
 * to ensure it is preserved in task switches
 */
#if defined(__propeller__) && defined(__GNUC__)
#define _TLSDECL(x) extern __attribute__((cogmem)) x
#else
#define _TLSDECL(x) extern x
#endif

_TLSDECL( struct _TLS *_TLS );

#endif
