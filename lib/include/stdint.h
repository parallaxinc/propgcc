#ifndef _STDINT_H
#define _STDINT_H

#include <compiler.h>

typedef unsigned char uint8_t;
typedef signed char   int8_t;

typedef unsigned short uint16_t;
typedef short          int16_t;

#if _INT_SIZE == 4
typedef unsigned int  uint32_t;
typedef int           int32_t;
#elif _LONG_SIZE == 4
typedef unsigned long uint32_t;
typedef long          int32_t;
#else
#error "compiler not supported"
#endif

typedef unsigned long long uint64_t;
typedef long long          int64_t;

typedef int8_t         int_least8_t;
typedef int16_t        int_least16_t;
typedef int32_t        int_least32_t;
typedef int64_t        int_least64_t;
typedef uint8_t        uint_least8_t;
typedef uint16_t       uint_least16_t;
typedef uint32_t       uint_least32_t;
typedef uint64_t       uint_least64_t;

/* assume "int" is the fastest integer type */
typedef int            int_fast8_t;
typedef unsigned int   uint_fast8_t;
typedef int            int_fast16_t;
typedef unsigned int   uint_fast16_t;
#if _INT_SIZE >= 4
typedef int            int_fast32_t;
typedef unsigned int   uint_fast32_t;
#else
typedef int32_t        int_fast32_t;
typedef uint32_t       uint_fast32_t;
#endif
typedef int64_t        int_fast64_t;
typedef uint64_t       uint_fast64_t;

/* assume "long" will hold a pointer */
typedef long           intptr_t;
typedef unsigned long  uintptr_t;

/* and 64 bits is as wide as we go */
typedef int64_t        intmax_t;
typedef uint64_t       uintmax_t;

#define INT8_MIN  (-128)
#define INT8_MAX  (127)
#define UINT8_MAX (255)

#define INT16_MIN  (-32768)
#define INT16_MAX  (32767)
#define UINT16_MAX (65535)

#define INT32_MIN (-2147483648L)
#define INT32_MAX (2147483647L)
#define UINT32_MAX (4294967295UL)

#define INT64_MIN (-9223372036854775808LL)
#define INT64_MAX (9223372036854775807LL)
#define UINT64_MAX (18446744073709551615ULL)

#define INT_LEAST8_MIN INT8_MIN
#define INT_LEAST8_MAX INT8_MAX
#define UINT_LEAST8_MAX UINT8_MAX
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST16_MAX INT16_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST32_MAX INT32_MAX
#define UINT_LEAST32_MAX UINT32_MAX

#if _INT_SIZE == 2
#define INT_FAST8_MIN  INT16_MIN
#define INT_FAST8_MAX  INT16_MAX
#define UINT_FAST8_MAX UINT16_MAX
#define INT_FAST16_MIN  INT16_MIN
#define INT_FAST16_MAX  INT16_MAX
#define UINT_FAST16_MAX UINT16_MAX
#else
#define INT_FAST8_MIN  INT32_MIN
#define INT_FAST8_MAX  INT32_MAX
#define UINT_FAST8_MAX UINT32_MAX
#define INT_FAST16_MIN  INT32_MIN
#define INT_FAST16_MAX  INT32_MAX
#define UINT_FAST16_MAX UINT32_MAX
#endif

#define INT_FAST32_MIN  INT32_MIN
#define INT_FAST32_MAX  INT32_MAX
#define UINT_FAST32_MAX UINT32_MAX

#define INT_FAST64_MIN  INT64_MIN
#define INT_FAST64_MAX  INT64_MAX
#define UINT_FAST64_MAX UINT64_MAX

#if _LONG_SIZE == 4
#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#else
#define INTPTR_MIN INT64_MIN
#define INTPTR_MAX INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#endif

#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MIN UINT64_MIN

/* C++ only gets these definitions if __STDC_LIMIT_MACROS is defined */
#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS)

#define PTRDIFF_MIN INTPTR_MIN
#define PTRDIFF_MAX INTPTR_MAX

#define SIZE_MAX    UINTPTR_MAX

#if _INT_SIZE == 2
#define SIG_ATOMIC_MIN INT16_MIN
#define SIG_ATOMIC_MAX INT16_MAX
#else
#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX
#endif

#if _WCHAR_SIZE == 2
#define WCHAR_MIN 0
#define WCHAR_MAX UINT16_MAX
#define WINT_MIN  INT16_MIN
#define WINT_MAX  INT16_MAX
#else
#define WCHAR_MIN 0
#define WCHAR_MAX UINT32_MAX
#define WINT_MIN  INT32_MIN
#define WINT_MAX  INT32_MAX
#endif

#endif

#endif
