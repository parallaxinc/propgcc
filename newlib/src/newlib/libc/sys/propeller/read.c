#include <errno.h>
#include <reent.h>
#include "propdev.h"

static _ssize_t null_term_read(void *buf, size_t bytes);
static _ssize_t null_file_read(int fd, void *buf, size_t bytes);

_ssize_t (*_term_read_p)(void *buf, size_t bytes) = null_term_read;
_ssize_t (*_file_read_p)(int fd, void *buf, size_t bytes) = null_file_read;

_ssize_t _read_r(struct _reent *reent, int fd, void *buf, size_t bytes)
{
    _ssize_t cnt;
    switch (fd) {
    case 0:
        cnt = (*_term_read_p)(buf, bytes);
        break;
    case 1:
    case 2:
        reent->_errno = EBADF;
        cnt = -1;
        break;
    default:
        cnt = (*_file_read_p)(fd, buf, bytes);
        break;
    }
    return cnt;
}

static _ssize_t null_term_read(void *buf, size_t bytes)
{
    return -1;
}

static _ssize_t null_file_read(int fd, void *buf, size_t bytes)
{
    return -1;
}
