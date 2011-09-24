/*
 * data structures for exit and atexit
 */
#ifndef _SYS_EXIT_H
#define _SYS_EXIT_H

struct _atexit_handler {
  struct _atexit_handler *next;
  void (*func)(void);
};

extern struct _atexit_handler *__atexitlist;
#endif
