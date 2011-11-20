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

/*
 * default function for giving up CPU time
 */
static void
dummy(void)
{
}

void (*__yield_ptr)(void) = dummy;

/*
 * function for sleeping until a specific cycle is reached or passed
 */
void (*__napuntil_ptr)(unsigned int untilcycle) = 0;

