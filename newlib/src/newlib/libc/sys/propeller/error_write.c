#include <errno.h>
#include <reent.h>
#include "propdev.h"

int _serial_tx(int);

static int null_error_putc(int ch);

int (*_error_putc_p)(int ch) = _serial_tx; // BUG -- should be null_error_putc;

_ssize_t _error_write(const void *buf, size_t bytes)
{
    const char *cbuf = (const char *)buf;
    size_t i = 0;
    while (i < bytes)
        (*_error_putc_p)(cbuf[i++]);
    return bytes;
}

int _errbyte(int ch)
{
    return (*_error_putc_p)(ch);
}

static int null_error_putc(int ch)
{
    return 1;
}
