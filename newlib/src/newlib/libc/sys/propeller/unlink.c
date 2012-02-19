#include <errno.h>
#include <reent.h>

static int null_file_unlink(const char *pathname);

int (*_file_unlink_p)(const char *pathname) = null_file_unlink;

int _unlink_r(struct _reent *reent, const char *pathname)
{
    return (*_file_unlink_p)(pathname);
}

static int null_file_unlink(const char *pathname)
{
    return -1;
}
