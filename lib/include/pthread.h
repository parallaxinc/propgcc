/*
 * @pthread.h
 * Implementation of pthread functions
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <sys/thread.h>
#include <sys/size_t.h>
#include <setjmp.h>

/* minimum stack size */
#define PTHREAD_STACK_MIN 64
#define _PTHREAD_STACK_DEFAULT 512

/* flags for the pthread "flags" field */
#define _PTHREAD_DETACHED   0x0001
#define _PTHREAD_TERMINATED 0x8000

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

/* a pthread_t is just a pointer to the thread state structure */
typedef _thread_state_t _pthread_state_t;
typedef _thread_queue_t _pthread_queue_t;

typedef _pthread_state_t *pthread_t;

typedef struct pthread_attr_t {
  size_t stksiz;       /* stack size */
  void *stack;        /* pointer to base of stack, NULL to allocate one */
  unsigned int flags; /* flags to start with */
} pthread_attr_t;

/* a mutex is just an integer with an associated queue */
typedef struct pthread_mutex_t {
  atomic_t cnt;
  _pthread_queue_t queue;
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { 0, 0 }

/* mutex attributes are not implemented right now */
typedef int pthread_mutexattr_t;

/* a lock for pthreads data structures */
extern atomic_t __pthreads_lock;
#define __lock_pthreads() __lock(&__pthreads_lock)
#define __unlock_pthreads() __unlock(&__pthreads_lock)

/*
 * some internal functions
 */
void _pthread_sleep(_pthread_queue_t *queue);
void _pthread_sleep_with_lock(_pthread_queue_t *queue);
int _pthread_wake(_pthread_queue_t *queue);
int _pthread_wakeall(_pthread_queue_t *queue);
void _pthread_free(_pthread_state_t *thr);
#if 0
_pthread_state_t *_pthread_ptr(pthread_t thread);
_pthread_state_t *_pthread_self()
#else
#define _pthread_ptr(thread) (thread)
#define _pthread_self() _TLS
#endif

/*
 * functions to manipulate the pthread attributes
 */
int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getdetachstate(pthread_attr_t *attr, int *detachstate);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);

/*
 * pthread functions
 */

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
		    void *(startfunc)(void*), void *arg);
void pthread_exit(void *);
int pthread_join(pthread_t thread, void **result_ptr);
int pthread_detach(pthread_t thread);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);

void pthread_yield(void);
pthread_t pthread_self(void);

/*
 * set processor affinity
 */

/* set a mask of which cogs this thread can run on */
void pthread_set_cog_affinity_np(pthread_t *thread, unsigned short cogmask);

/* set the current thread to run on the current cog */
void pthread_set_affinity_thiscog_np(void);

#define pthread_set_cog_affinity_np(thread, mask) \
  do { thread->affinity = ~(mask); } while (0)

#define pthread_set_affinity_thiscog_np() pthread_set_cog_affinity_np(_TLS, __this_cpu_mask())

#endif
