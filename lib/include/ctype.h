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

#define isalnum(c)      (__ctype[(unsigned char)(c)]&(_CTu|_CTl|_CTd))
#define isalpha(c)      (__ctype[(unsigned char)(c)]&(_CTu|_CTl))
#define isblank(c)      (__ctype[(unsigned char)(c)]&(_CTb))
#define iscntrl(c)      (__ctype[(unsigned char)(c)]&_CTc)
#define isdigit(c)      (__ctype[(unsigned char)(c)]&_CTd)
#define isgraph(c)      (!(__ctype[(unsigned char)(c)]&(_CTc|_CTs)) && (__ctype[(unsigned char)(c)]))
#define islower(c)      (__ctype[(unsigned char)(c)]&_CTl)
#define isprint(c)      (!(__ctype[(unsigned char)(c)]&_CTc) && (__ctype[(unsigned char)(c)]))
#define ispunct(c)      (__ctype[(unsigned char)(c)]&_CTp)
#define isspace(c)      (__ctype[(unsigned char)(c)]&_CTs)
#define isupper(c)      (__ctype[(unsigned char)(c)]&_CTu)
#define isxdigit(c)     (__ctype[(unsigned char)(c)]&_CTx)


#if defined(__cplusplus)
}
#endif

#endif
