#ifndef _STRING_H_
#define _STRING_H_

#ifndef _SIZE_T_DEFINED
#define _SIZE_T_DEFINED unsigned int
typedef _SIZE_T_DEFINED size_t;
#endif

#ifndef NULL
#define NULL (0)
#endif

size_t strlen(const char *s);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *src, size_t n);
char * strcpy(char *dest, const char *src);
char * strncpy(char *dest, const char *src, size_t n);

void *memcpy(void *dest, const void *src, size_t n);

#endif
