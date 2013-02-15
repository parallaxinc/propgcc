/*
# #########################################################
# This file contains the interface code for the DOSFS FAT
# file sytem driver.  The code is separated into two main
# sections, which consist of the standard file I/O driver
# and the basic file routines that call the DOSFS fucntions.
#   
# Written by Dave Hein with contributions from other GCC
# development team members.
# Copyright (c) 2011 Parallax, Inc.
# MIT Licensed
# #########################################################
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/driver.h>
#include <compiler.h>
#include <errno.h>
#include <propeller.h>
#include <sys/sd.h>
#include "../drivers/sd_internal.h"

void dfs_resolve_path(const char *fname, char *path);

int rmdir(const char *path1)
{
    uint8_t *ptr;
    DIRENT dirent;
    DIRINFO dirinfo;
    FILEINFO fileinfo;
    int dirsector, diroffset;
    char path[MAX_PATH];

    if (!dfs_mountflag && dfs_mount_defaults() != DFS_OK)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path((char *)path1, path);

    dirinfo.scratch = dfs_scratch;

    // Open the directory
    if (DFS_OK != DFS_OpenDir(&dfs_volinfo, (uint8_t *)path, &dirinfo))
    {
        // Try opening as a file
        if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (uint8_t *)path, DFS_READ, dfs_scratch, &fileinfo))
            errno = ENOTDIR;
        else
            errno = ENOENT;
        return -1;
    }

    // Check if directory is empty
    while (DFS_OK == DFS_GetNext(&dfs_volinfo, &dirinfo, &dirent))
    {
        if (dirent.name[0] && !(strncmp((char*)dirent.name, ".          ", 11) == 0 ||
                                strncmp((char*)dirent.name, "..         ", 11) == 0))
        {
            errno = ENOTEMPTY;
            return -1;
        }
    }

    // Open the directory as a file
    if (DFS_OK != DFS_OpenFile(&dfs_volinfo, (uint8_t *)path, DFS_DIRECTORY, dfs_scratch, &fileinfo))
    {
        errno = ENOENT;
        return -1;
    }

    // Remove the directory attribute so we can delete it
    dirsector = fileinfo.dirsector;
    diroffset = fileinfo.diroffset;
    ptr = &dfs_scratch[diroffset*32];
    DFS_ReadSector(0, dfs_scratch, dirsector, 1);
    ptr[11] = 0;
    DFS_WriteSector(0, dfs_scratch, dirsector, 1);

    // Delete the directory
    DFS_UnlinkFile(&dfs_volinfo, (uint8_t *)path, dfs_scratch);

    return DFS_OK;
}

/*
+--------------------------------------------------------------------
|  TERMS OF USE: MIT License
+--------------------------------------------------------------------
Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files
(the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
+------------------------------------------------------------------
*/
