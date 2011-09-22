#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include "propdev.h"
#include "dosfs.h"

#define FS_DEBUG

_fs_state_t _fs_state = { FALSE };

int InitFileIO(void)
{
	_file_read_p = _file_read;
	_file_write_p = _file_write;
    _file_close_p = _file_close;
    _file_unlink_p = _file_unlink;
	return 0;
}

int MountFS(void)
{
    uint8_t active, ptype;
    uint32_t start, psize, err;
    int i;

#ifdef FS_DEBUG
	iprintf("GetPtnStart:\r\n");
#endif
	start = DFS_GetPtnStart(0, _fs_state.scratch, 0, &active, &ptype, &psize);
#ifdef FS_DEBUG
	iprintf(" -->%ld\r\n", start);
#endif
	if (start == DFS_ERRMISC)
        return FALSE;
#ifdef FS_DEBUG
    iprintf("active %d, ptype %d, psize %lx, start %lx\r\n", active, ptype, psize, start);
#endif

#ifdef FS_DEBUG
	iprintf("GetVolInfo:\r\n");
#endif
	err = DFS_GetVolInfo(0, _fs_state.scratch, start, &_fs_state.vinfo);
#ifdef FS_DEBUG
	iprintf(" -->%ld\r\n", err);
#endif
	if (err != DFS_OK)
        return FALSE;

#ifdef FS_DEBUG
	iprintf("Volume label '%-11.11s'\r\n", _fs_state.vinfo.label);
	iprintf("%d sector/s per cluster, %d reserved sector/s, volume total %ld sectors.\r\n",
                _fs_state.vinfo.secperclus,
                _fs_state.vinfo.reservedsecs,
                _fs_state.vinfo.numsecs);
	iprintf("%ld sectors per FAT, first FAT at sector #%ld, root dir at #%ld.\r\n",
                _fs_state.vinfo.secperfat,
                _fs_state.vinfo.fat1,
                _fs_state.vinfo.rootdir);
	iprintf("(For FAT32, the root dir is a CLUSTER number, FAT12/16 it is a SECTOR number)\r\n");
	iprintf("%d root dir entries, data area commences at sector #%ld.\r\n",
                _fs_state.vinfo.rootentries,
                _fs_state.vinfo.dataarea);
	iprintf("%ld clusters (%ld bytes) in data area, filesystem IDd as ",
                _fs_state.vinfo.numclusters,
                _fs_state.vinfo.numclusters * _fs_state.vinfo.secperclus * SECTOR_SIZE);
	if (_fs_state.vinfo.filesystem == FAT12)
		iprintf("FAT12.\r\n");
	else if (_fs_state.vinfo.filesystem == FAT16)
		iprintf("FAT16.\r\n");
	else if (_fs_state.vinfo.filesystem == FAT32)
		iprintf("FAT32.\r\n");
	else {
		iprintf("[unknown]\r\n");
        return FALSE;
    }
#endif

    for (i = 0; i < _FS_MAX_FILES; ++i)
        _fs_state.files[i].inUse = FALSE;

	_fs_state.initialized = TRUE;

    return TRUE;
}
