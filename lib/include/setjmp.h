#ifndef _SETJMP_H
#define _SETJMP_H

#ifdef __propeller__
#define _JBLEN 9
#else
#error "unknown machine type"
#endif

#ifndef _JBTYPE
#define _JBTYPE unsigned long
#endif

typedef _JBTYPE jmp_buf[_JBLEN];

void longjmp(jmp_buf env, int val);
int  setjmp(jmp_buf env);

#endif
