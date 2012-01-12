#ifndef _WCHAR_H
#define _WCHAR_H

#define _NEED_WINT_T
#include <sys/wchar_t.h>
#include <sys/size_t.h>
#include <sys/null.h>

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

  wchar_t *wcscpy(wchar_t *__restrict s1, const wchar_t *__restrict s2);
  wchar_t *wcscat(wchar_t *__restrict s1, const wchar_t *__restrict s2);
  size_t   wcslen(const wchar_t *s);

#if defined(__cplusplus)
}
#endif


#endif
