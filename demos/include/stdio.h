#ifndef _STDIO_H_
#define _STDIO_H_

/* the transmit function (used to put a single character)
 * returns 1 on success, 0 on failure
 */
extern int (*_putc)(int c);


#define putchar(c) (_putc(c) ? c : 0);

/* external definitions */
extern int printf(const char *fmt, ...);

#endif
