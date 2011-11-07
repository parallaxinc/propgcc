#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <sys/thread.h>
#include <sys/size_t.h>

/*
 * @pthread.h
 * Implementation of pthread functions
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

/* flags for the pthread "flags" field */
#define _PTHREAD_ALLOCATED  0x0001
#define _PTHREAD_FREE_STACK 0x0002
#define _PTHREAD_TERMINATED 0x0004
#define _PTHREAD_DETACHED   0x0008

typedef struct _pthread_status_t {
  char cogid;           /* cog this thread is running on */
  char thrid;           /* individual thread within cog (not currently used) */
  unsigned short flags; /* various thread flags */
  void *stackbase;      /* base address for stack */

  /* function to call */
  void *(*startfunc)(void *);
  /* argument for it (later used for result) */
  void *arg;

  /* thread local variables for this thread */
  struct _TLS TLSdata;

} _pthread_status_t;

#define _NUM_PTHREADS 8
extern _pthread_status_t _PTHREAD[_NUM_PTHREADS];
#define _POSIX_THREAD_THREADS_MAX _NUM_PTHREADS

/* index into the array of _pthread_status structs */
typedef int pthread_t;

typedef struct pthread_attr_t {
  size_t stksiz;       /* stack size */
  void *stack;        /* pointer to base of stack, NULL to allocate one */
} pthread_attr_t;

extern volatile int _pthreads_lock;
#define _lock_pthreads() _lock(&_pthreads_lock)
#define _unlock_pthreads() _unlock(&_pthreads_lock)

/*
 * functions to manipulate the pthread attributes
 */
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);

/*
 * pthread functions
 */
_pthread_status_t *_pthread_ptr(pthread_t thread);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		    void *(startfunc)(void*), void *arg);
void pthread_exit(void *);
int pthread_join(pthread_t thread, void **result_ptr);
int pthread_detach(pthread_t thread);

#if 0
int pthread_yield(void);
#else
#define pthread_yield()
#endif

#endif
