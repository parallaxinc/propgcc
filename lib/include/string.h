#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>

size_t strlen(const char *s);
char * strcat(char *dest, const char *src);
char * strncat(char *dest, const char *src, size_t n);
char * strcpy(char *dest, const char *src);
char * strncpy(char *dest, const char *src, size_t n);

int    strcmp(const char *s1, const char *s2);
int    strncmp(const char *s1, const char *s2, size_t n);

char *strchr(const char *, int);
char *strrchr(const char *, int);

char *strtok(char *str, const char *delim);
size_t strspn(const char *, const char *);
size_t strcspn(const char *, const char *);

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
void *memchr(const void *s, int c, size_t n);
int   memcmp(const void *s1, const void *s2, size_t n);

size_t strxfrm(char *dest, const char *src, size_t n);
int    strcoll(const char *s1, const char *s2);

char *strerror(int err);

#endif
