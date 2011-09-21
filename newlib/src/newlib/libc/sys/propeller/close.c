#include <errno.h>
#include <reent.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

int _close_r(struct _reent *reent, int fd)
{
    int i = fd - _FS_BASE_FD;
    if (!_fs_state.initialized) {
        reent->_errno = EIO;
        return -1;
    }
    if (i < 0 || i >= _FS_MAX_FILES) {
        reent->_errno = EBADF;
        return -1;
    }
    if (!_fs_state.files[i].inUse) {
        reent->_errno = EINVAL;
        return -1;
    }
    _fs_state.files[i].inUse = FALSE;
    return 0;
}
