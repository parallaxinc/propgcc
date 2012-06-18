#include <errno.h>
#include <reent.h>
#include "propdev.h"

#define DEL 0x7f

static int null_term_getc(void);

int (*_term_getc_p)(void) = null_term_getc;

_ssize_t _term_read(void *buf, size_t nbytes)
{
    char *cbuf = (char *)buf;
    int i = 0;
    while (i < nbytes) {
        int ch;
        if ((ch = (*_term_getc_p)()) == -1)
            return -1;
        else if ((ch == '\r') || (ch == '\n')) {
            cbuf[i++] = '\n';
            (*_term_putc_p)('\r');
            (*_term_putc_p)('\n');
            break;
        }
        else if (ch == '\b' || ch == DEL) {
            if (i > 0) {
                (*_term_putc_p)('\b');
                (*_term_putc_p)(' ');
                (*_term_putc_p)('\b');
                --i;
            }
        }
        else {
            cbuf[i++] = ch;
            (*_term_putc_p)(ch);
        }
    }
    return i;
}

int _inbyte(void)
{
    return (*_term_getc_p)();
}

static int null_term_getc(void)
{
    return -1;
}
