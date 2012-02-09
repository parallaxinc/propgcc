#ifndef _CTYPE_H
#define _CTYPE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/ctype.h>

  extern int isalnum(int c);
  extern int isalpha(int c);
  extern int isblank(int c);
  extern int iscntrl(int c);
  extern int isdigit(int c);
  extern int isgraph(int c);
  extern int islower(int c);
  extern int isprint(int c);
  extern int ispunct(int c);
  extern int isspace(int c);
  extern int isupper(int c);
  extern int isxdigit(int c);

  extern int tolower(int c);
  extern int toupper(int c);


#define __isctype(c, x) (__ctype_get(c) & x)

#define isalnum(c)     __isctype(c, _CTalnum)
#define isalpha(c)     __isctype(c, _CTalpha)
#define isblank(c)     __isctype(c, _CTb)
#define iscntrl(c)     __isctype(c, _CTc)
#define isdigit(c)     __isctype(c, _CTd)
#define isgraph(c)     (!__isctype(c, (_CTc|_CTs)) && (__ctype_get(c) != 0))
#define islower(c)     __isctype(c, _CTl)
#define isprint(c)     (!__isctype(c, (_CTc)) && (__ctype_get(c) != 0))
#define ispunct(c)     __isctype(c, _CTp)
#define isspace(c)     __isctype(c, _CTs)
#define isupper(c)     __isctype(c, _CTu)
#define isxdigit(c)    __isctype(c, _CTx)


#if defined(__cplusplus)
}
#endif

#endif
