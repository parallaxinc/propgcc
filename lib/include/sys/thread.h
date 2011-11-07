#ifndef _SYS_THREAD_H
#define _SYS_THREAD_H

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
 * in the TLS structure
 */
struct _TLS {
  int errno;
  char *strtok_scanpoint;
  unsigned long rand_seed;
  struct tm time_temp;
  char ctime_buf[32];
  int  thread_id; /* thread ID for this thread */
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

_TLSDECL( struct _TLS *_TLS );

/*
 * start a new cog thread running C code
 * (as if "func(arg)" was called in that thread)
 * tls is a pointer to the thread local storage area to use
 */
int _start_cog_thread(void *stacktop, void (*func)(void *), void *arg, struct _TLS *tls);

/*
 * function to do a "compare and swap" operation on a memory location
 * this is protected with the _C_LOCK lock, so it should be atomic
 * (as long as all threads use this function to access the location)
 * if *ptr == checkval, then set *ptr to newval; returns original
 * value of *ptr
 */
#ifdef __GNUC__
__attribute__((native))
#endif
int _CMPSWAPSI(int newval, int checkval, volatile int *ptr);

#if (defined(__PROPELLER_LMM__) || defined(__PROPELLER_COG__))
#define _lock(val) while (_CMPSWAPSI(1, 0, val) != 0) ;
#else
#define _lock(val) *val = 1
#endif

#define _unlock(val) *val = 0

#endif
