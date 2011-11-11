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
#include "dfs.h"

VOLINFO dfs_volinfo;
int dfs_mountflag;
extern __attribute__((section(".hub"))) uint8_t dfs_scratch[512];
char dfs_currdir[MAX_PATH];

int dfs_stdio_errno(int errnum);
void dfs_resolve_path(const char *fname, char *path);

extern _Driver _SimpleSerialDriver;
extern _Driver _FileDriver;

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */
_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  &_FileDriver,
  NULL
};

int dfs_rmdir(const char *path1)
{
    uint8_t *ptr;
    DIRENT dirent;
    DIRINFO dirinfo;
    FILEINFO fileinfo;
    int dirsector, diroffset;
    char path[MAX_PATH];

    if (!dfs_mountflag)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path((char *)path1, path);

    dirinfo.scratch = dfs_scratch;

    // Open the directory
    if (DFS_OK != DFS_OpenDir(&dfs_volinfo, path, &dirinfo))
    {
        // Try opening as a file
        if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (char *)path, DFS_READ, dfs_scratch, &fileinfo))
            errno = ENOTDIR;
        else
            errno = ENOENT;
        return -1;
    }

    // Check if directory is empty
    while (DFS_OK == DFS_GetNext(&dfs_volinfo, &dirinfo, &dirent))
    {
        if (dirent.name[0])
        {
            errno = ENOTEMPTY;
            return -1;
        }
    }

    // Open the directory as a file
    if (DFS_OK != DFS_OpenFile(&dfs_volinfo, (char *)path, DFS_DIRECTORY, dfs_scratch, &fileinfo))
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
    DFS_UnlinkFile(&dfs_volinfo, (char *)path, dfs_scratch);

    return DFS_OK;
}

int dfs_mkdir(const char *path1)
{
    PFILEINFO fileinfo;
    char path[MAX_PATH];

    if (!dfs_mountflag)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path((char *)path1, path);

    fileinfo = malloc(sizeof(FILEINFO));

    if (!fileinfo)
    {
        errno = ENOMEM;
        return -1;
    }

    if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (char *)path, DFS_DIRECTORY, dfs_scratch, fileinfo))
    {
        free(fileinfo);
        errno = EEXIST;
        return -1;
    }

    if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (char *)path, DFS_DIRECTORY | DFS_WRITE, dfs_scratch, fileinfo))
    {
        int i;
        int secperclus = dfs_volinfo.secperclus;
        int firstcluster = fileinfo->firstcluster;
        int firstsector = dfs_volinfo.dataarea + ((firstcluster - 2) * secperclus);

        for (i = 0; i < 512; i += 32) dfs_scratch[i] = 0;
        for (i = 0; i < secperclus; i++)
            DFS_WriteSector(0, dfs_scratch, firstsector++, 1);
    }
    else
    {
        free(fileinfo);
        errno = EIO;
        return -1;
    }

    free(fileinfo);

    return DFS_OK;
}

PDIRINFO dfs_opendir(char *dirname1)
{
    int i;
    int retval;
    PDIRINFO dirinfo;
    char dirname[MAX_PATH];

    if (!dfs_mountflag)
    {
        errno = EIO;
        return 0;
    }

    dirinfo = malloc(sizeof(DIRINFO) + sizeof(DIRENT) + SECTOR_SIZE);

    if (!dirinfo)
    {
        errno = ENOMEM;
        return 0;
    }

    dfs_resolve_path((char *)dirname1, dirname);
    dirinfo->scratch = ((void *)dirinfo) + sizeof(DIRINFO) + sizeof(DIRENT);
    retval = DFS_OpenDir(&dfs_volinfo, dirname, dirinfo);

    if (retval == DFS_OK)
        return dirinfo;

    free(dirinfo);

    errno = dfs_stdio_errno(retval);

    return 0;
}

void dfs_closedir(PDIRINFO dirinfo)
{
    if (dirinfo)
        free(dirinfo);
}

PDIRENT dfs_readdir(PDIRINFO dirinfo)
{
    int i;
    int retval;
    PDIRENT dirent = ((void *)dirinfo) + sizeof(DIRINFO);

    while (1)
    {
        retval = DFS_GetNext(&dfs_volinfo, dirinfo, dirent);
        if (retval != DFS_OK) return 0;
        if (dirent->name[0]) break;
    }

    return dirent;
}

int dfs_chdir(const char *path1)
{
    int len;
    int retval;
    char path[MAX_PATH];
    DIRINFO dirinfo;

    if (!dfs_mountflag)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path(path1, path);

    dirinfo.scratch = dfs_scratch;

    len = strlen(path);
    if (len > 1 && path[len-1] == '/') path[len-1] = 0;

    // Make sure it's a valid directory
    if (DFS_OK == (retval = DFS_OpenDir(&dfs_volinfo, path, &dirinfo)))
    {
        strcpy(dfs_currdir, path);
        return DFS_OK;
    }

    errno = dfs_stdio_errno(retval);

    return -1;
}

char *dfs_getcd(void)
{
    return dfs_currdir;
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
