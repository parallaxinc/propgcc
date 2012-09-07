/**
 * @file include/pthread.h
 * @brief Provides API for implementation of pthread functions.
 *
 * POSIX threads are a set of functions that support applications
 * with requirements for multiple flows of control, called threads,
 * within a process.  Multithreading is used to improve the
 * performance of a program.
 *
 * The pthread module provides a subset of the POSIX standard
 * pthread library for allowing multiple threads to run on
 * multiple COG instances of the PropellerGCC LMM interpreter.
 * Multiple threads can also run on an XMM interpreter, but
 * only one XMM interpreter can run at any given time.
 *
 * @pre It is very important to understand that pthreads provide
 * a cooperative non-preemptive multithreading system.
 * The consequences are that certain rules will apply. I.E.
 * @li The Propeller waitcnt() function can not be used.
 * @li Delays can only be introduced with usleep() and its variations.
 * @li Standard printf() and friends must be used.
 * @li Do not use -Dprintf=__simple_printf with pthreads.
 * @li Threads must not hog the LMM COG kernel; they must yield.
 * @li Gobal data must be protected by mutex/semaphore.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed.
 */

#ifndef _PTHREAD_H
#define _PTHREAD_H

#include <sys/thread.h>
#include <sys/size_t.h>
#include <sys/null.h>
#include <setjmp.h>

/** @brief Minimum stack size for a thread. */
#define PTHREAD_STACK_MIN 64
/** @brief Default stack size for a thread. */
#define _PTHREAD_STACK_DEFAULT 512

/** @brief Detached flag for the pthread "flags" field */
#define _PTHREAD_DETACHED   0x0001
/** @brief Terminated flag for the pthread "flags" field */
#define _PTHREAD_TERMINATED 0x8000

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 1

/* a pthread_t is just a pointer to the thread state structure */
typedef _thread_state_t _pthread_state_t;
typedef _thread_queue_t _pthread_queue_t;

/**
 * @brief The pthread_t typedef is used as the "handle" for threads and functions.
 */
typedef _pthread_state_t *pthread_t;

/**
 * @brief The pthread_attr_t struct TODO
 */
typedef struct pthread_attr_t {
/** @brief stack size */
  size_t stksiz;       /* stack size */
/** @brief pointer to base of stack, NULL to allocate one */
  void *stack;        /* pointer to base of stack, NULL to allocate one */
/** @brief flags to start with */
  unsigned int flags; /* flags to start with */
} pthread_attr_t;

/**
 * @brief The pthread_mutex_t TODO
 */
/* a mutex is just an integer with an associated queue */
typedef struct pthread_mutex_t {
  atomic_t cnt;
  _pthread_queue_t queue;
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { 0, 0 }

/** @brief Mutex attributes are not implemented. */
typedef int pthread_mutexattr_t;

/** @brief Lock for pthreads data structures */
extern atomic_t __pthreads_lock;
#define __lock_pthreads() __lock(&__pthreads_lock)
#define __unlock_pthreads() __unlock(&__pthreads_lock)

/** @brief Condition variables */
typedef struct pthread_cond_t {
  _pthread_queue_t queue;
} pthread_cond_t;

/** @brief Condition variable attributes are not implemented. */
typedef int pthread_condattr_t;

/** @brief Destructor function associated with thread specific data */
typedef void (*__pthread_destruct_func)(void *);

/** @brief Maximum number of keys for per-thread data */
#define PTHREAD_KEYS_MAX _THREAD_KEYS_MAX

/** @brief Type for per-thread data keys */
typedef unsigned char pthread_key_t;

/** @brief Table of destructors for thread specific data */
extern __pthread_destruct_func __pdestruct[PTHREAD_KEYS_MAX];
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
int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize);
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
int pthread_mutex_destroy(pthread_mutex_t *mutex);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int pthread_cond_destroy(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond);
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);

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

#define pthread_getspecific(i) (_TLS->thread_data[i])
#define pthread_setspecific(i, x) (_TLS->thread_data[i] = (x))

#endif
