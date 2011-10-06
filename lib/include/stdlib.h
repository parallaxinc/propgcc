#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/size_t.h>
#include <sys/null.h>
#include <compiler.h>

double atof(const char *);
int    atoi(const char *);
long   atol(const char *);

long double strtold(const char *nptr, char **endptr);
double strtod(const char *nptr, char **endptr);
float  strtof(const char *nptr, char **endptr);

long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);

#define RAND_MAX 0x3fff
int rand(void);
void srand(unsigned int seed);

void *malloc(size_t n);
void *calloc(size_t, size_t);
void *realloc(void *, size_t);
void free(void *);

int atexit(void (*func)(void));
_NORETURN void exit(int status);
_NORETURN void abort(void);
_NORETURN void _Exit(int status);

_CONST int abs(int i);
_CONST long labs(long l);
_CONST long long llabs(long long ll);

typedef struct {
  int quot, rem;
} div_t;

typedef struct {
  long int quot, rem;
} ldiv_t;

typedef struct {
  long long quot, rem;
} lldiv_t;

div_t div(int num, int denom);
ldiv_t ldiv(long num, long denom);
lldiv_t lldiv(long long num, long long denom);

void qsort(void *base, size_t nmemb, size_t size, int (*compare)(const void *, const void *));
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
	      int (*compare)(const void *, const void *));
#endif
