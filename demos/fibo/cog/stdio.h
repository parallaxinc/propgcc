#ifndef _STDIO_H_
#define _STDIO_H_

extern int printf(const char *fmt, ...);

extern void _start_serial(int rxpin, int txpin, int buadrate);

#endif
