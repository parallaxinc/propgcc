/*
 * note, this header file may be included multiple times, with and without
 * _NEED_WINT_T defined first
 * if _NEED_WINT_T is defined, we define wint_t
 */

#ifndef _WCHAR_T_DEFINED
#ifndef _WCHAR_T_TYPE
#define _WCHAR_T_TYPE unsigned int
#endif
#define _WCHAR_T_DEFINED _WCHAR_T_TYPE
typedef _WCHAR_T_DEFINED wchar_t;
#endif

#if defined(_NEED_WINT_T) && !defined(_WINT_T_DEFINED)
#define _WINT_T_DEFINED int
typedef _WINT_T_DEFINED wint_t;
#endif
