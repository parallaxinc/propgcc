#include <reent.h>
#include <errno.h>
#include <fcntl.h>
#include "propdev.h"

extern _fs_state_t _fs_state;

int _open_r(struct _reent *reent, const char *path, int access, int mode)
{
    int dfs_mode, i;
	if (!_fs_state.initialized) {
		if (!MountFS()) {
            reent->_errno = ENODEV;
			return -1;
        }
	}
    switch (access & O_ACCMODE) {
    case O_RDONLY:
        dfs_mode = DFS_READ;
        break;
    case O_WRONLY:
        dfs_mode = DFS_WRITE;
        break;
    default:
        reent->_errno = EINVAL;
        return -1;
    }
    for (i = 0; i < _FS_MAX_FILES; ++i)
        if (!_fs_state.files[i].inUse)
            break;
    if (i >= _FS_MAX_FILES) {
        reent->_errno = ENFILE;
        return -1;
    }
    if (DFS_OpenFile(&_fs_state.vinfo, (char *)path, dfs_mode, _fs_state.scratch, &_fs_state.files[i].file) != DFS_OK) {
        reent->_errno = ENOENT;
        return -1;
    }
    _fs_state.files[i].inUse = TRUE;
    return _FS_BASE_FD + i;
}

