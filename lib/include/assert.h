/* it's actually OK to include this file multiple times; we may
 * change the value of NDEBUG in between, too
 */
#include <compiler.h>

_NORETURN void abort(void);

#undef assert

extern void __eprintf( const char *expr, unsigned long line, const char *filename);

#if defined(NDEBUG)
#define assert(cond) (void)(0)
#else
#define assert(cond) \
  ((cond) ? 0 : (__eprintf(#cond,(long)(__LINE__), __FILE__), abort(), 0))
#endif
