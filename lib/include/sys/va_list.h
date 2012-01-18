#ifndef _va_list_defined
#define _va_list_defined

#if defined(__GNUC__)
typedef __builtin_va_list __va_list;
#else
#error "need a definition for _va_list"
#endif

#endif
