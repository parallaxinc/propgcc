#include <errno.h>
#include <reent.h>
#include "propdev.h"

static int null_error_putc(int ch);

int (*_error_putc_p)(int ch) = null_error_putc;

_ssize_t _error_write(const void *buf, size_t bytes)
{
    const char *cbuf = (const char *)buf;
    size_t i = 0;
    while (i < bytes)
        (*_error_putc_p)(cbuf[i++]);
    return bytes;
}

static int null_error_putc(int ch)
{
    return 1;
}
