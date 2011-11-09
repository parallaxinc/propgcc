#include <errno.h>

/*
 * default thread local variable
 */
static _thread_state_t default_thread;

#if defined(__propeller__) && defined(__GNUC__)
__attribute__((cogmem))
_thread_state_t *_TLS __attribute__((section(".kernel"))) = &default_thread;
#else
_thread_state_t *_TLS = &default_thread;
#endif
