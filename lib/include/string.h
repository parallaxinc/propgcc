#ifndef _STRING_H
#define _STRING_H

#include <sys/size_t.h>
#include <sys/null.h>

size_t strlen(const char *s);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *src, size_t n);
char * strcpy(char *dest, const char *src);
char * strncpy(char *dest, const char *src, size_t n);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);

#endif
