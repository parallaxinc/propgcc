#ifndef _SYS_THREAD_H
#define _SYS_THREAD_H

#include <sys/jmpbuf.h>

#ifndef _STRUCT_TM_DEFINED
#define _STRUCT_TM_DEFINED
/* time representing broken down calendar time */
struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year; /* years since 1900 */
  int tm_wday; /* days since Sunday */
  int tm_yday; /* days since January 1 */
  int tm_isdst; /* if > 0, DST is in effect, if < 0 info is not known */
};
#endif

/*
 * thread local storage
 * the library should not keep anything in global or
 * static variables; all such values should be placed
 * in the _thread structure
 */
typedef struct _thread _thread_state_t;
typedef _thread_state_t * volatile _thread_queue_t;

struct _thread {
  /* threads may sleep on queues */
  _thread_state_t *queue_next;
  _thread_queue_t *queue;

  int errno;
  char *strtok_scanpoint;

  unsigned long rand_seed;
  struct tm time_temp;
  char ctime_buf[32];
  _jmp_buf jmpbuf;       /* for thread context */
  short pri;             /* thread priority */
  unsigned short flags;  /* flags for this thread */

  /* re-use arg for the thread return value */
  void *arg;             /* thread argument */
  void *(*start)(void *);/* start function */
};

/*
 * we may want to define thread local storage in some special way
 * to ensure it is preserved in task switches
 */
#if defined(__propeller__) && defined(__GNUC__)
#define _TLSDECL(x) extern __attribute__((cogmem)) x
#else
#define _TLSDECL(x) extern x
#endif

_TLSDECL( _thread_state_t *_TLS );

/*
 * start a new cog thread running C code
 * (as if "func(arg)" was called in that thread)
 * tls is a pointer to the thread local storage area to use
 */
int _start_cog_thread(void *stacktop, void (*func)(void *), void *arg, _thread_state_t *tls);

/*
 * function to do a "compare and swap" operation on a memory location
 * this is protected with the _C_LOCK lock, so it should be atomic
 * (as long as all threads use this function to access the location)
 * if *ptr == checkval, then set *ptr to newval; returns original
 * value of *ptr
 */
#if !defined(__GNUC__)
#define __sync_bool_compare_and_swap(ptr, oldval, newval) \
  ((*ptr == oldval) && ((*ptr = newval),true)))
#define __sync_add_and_fetch(ptr, inc) \
  (*ptr += inc)
#endif

/* type for a volatile lock */
typedef volatile int atomic_t;

#if (defined(__PROPELLER_LMM__) || defined(__PROPELLER_COG__))
#define __trylock(ptr) __sync_bool_compare_and_swap(ptr, 0, 1)
#define __addlock(ptr, inc) __sync_add_and_fetch(ptr, inc)
#else
#define __trylock(val) (*val == 0 && (*val = 1) != 0)
#define __addlock(val, inc) (*val += inc)
#endif

#define __lock(val) while (!__trylock(val)) ;
#define __unlock(val) *val = 0

#endif
