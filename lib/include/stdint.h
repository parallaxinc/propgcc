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

#endif
