#include <errno.h>

/*
 * default thread local variable
 */
static struct _TLS default_TLS;

#if defined(__propeller__) && defined(__GNUC__)
__attribute__((cogmem))
struct _TLS *_TLS __attribute__((section(".kernel"))) = &default_TLS;
#else
struct _TLS *_TLS = &default_TLS;
#endif
