#include <errno.h>
#include <reent.h>
#include "propdev.h"

static _ssize_t null_term_write(const void *buf, size_t bytes);
static _ssize_t null_file_write(int fd, const void *buf, size_t bytes);

_ssize_t (*_term_write_p)(const void *buf, size_t bytes) = null_term_write;
_ssize_t (*_error_write_p)(const void *buf, size_t bytes) = null_term_write;
_ssize_t (*_file_write_p)(int fd, const void *buf, size_t bytes) = null_file_write;

_ssize_t _write_r(struct _reent *reent, int fd, const void *buf, size_t bytes)
{
    _ssize_t cnt;
    switch (fd) {
    case 0:
        reent->_errno = EBADF;
        cnt = -1;
        break;
    case 1:
        cnt = (*_term_write_p)(buf, bytes);
        break;
    case 2:
        cnt = (*_error_write_p)(buf, bytes);
        break;
    default:
        cnt = (*_file_write_p)(fd, buf, bytes);
        break;
    }
    return cnt;
}

static _ssize_t null_term_write(const void *buf, size_t bytes)
{
    return bytes;
}

static _ssize_t null_file_write(int fd, const void *buf, size_t bytes)
{
    return bytes;
}
