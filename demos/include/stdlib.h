#ifndef _STDLIB_H_
#define _STDLIB_H_

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED unsigned int
typedef _SIZE_T_DEFINED size_t;
#endif

#ifndef NULL
#define NULL (0)
#endif

void *malloc(size_t n);

#endif
