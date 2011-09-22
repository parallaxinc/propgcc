#include <reent.h>
#include <unistd.h>
#include <errno.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

off_t _lseek_r(struct _reent *reent, int fd, off_t offset, int whence)
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
    switch (whence) {
    case SEEK_SET:
        if (offset > _fs_state.files[i].file.filelen) {
            reent->_errno = EINVAL;
            return -1;
        }
        break;
    case SEEK_CUR:
        if ((offset += _fs_state.files[i].file.pointer) > _fs_state.files[i].file.filelen) {
            reent->_errno = EINVAL;
            return -1;
        }
        break;
    case SEEK_END:
        if (offset > _fs_state.files[i].file.filelen) {
            reent->_errno = EINVAL;
            return -1;
        }
        offset = _fs_state.files[i].file.filelen - offset;
        break;
    default:
        reent->_errno = EINVAL;
        return -1;
    }
    DFS_Seek(&_fs_state.files[i].file, offset, _fs_state.scratch);
    return 0;
}

