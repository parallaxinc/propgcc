#include <errno.h>
#include <reent.h>

static int null_file_close(int fd);

int (*_file_close_p)(int fd) = null_file_close;

int _close_r(struct _reent *reent, int fd)
{
    int ret;
    switch (fd) {
    case 0:
    case 1:
    case 2:
        reent->_errno = EIO;
        ret = -1;
    default:
        ret = (*_file_close_p)(fd);
        break;
    }
    return ret;
}

static int null_file_close(int fd)
{
    return -1;
}

