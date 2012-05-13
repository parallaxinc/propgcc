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
#include <sys/sd.h>
#include <compiler.h>
#include <errno.h>
#include <propeller.h>
#include <sys/stat.h>
#include "../drivers/sd_internal.h"

void dfs_resolve_path(const char *fname, char *path);

int mkdir(const char *path1, int mode)
{
    PFILEINFO fileinfo;
    char path[MAX_PATH];

    if (!dfs_mountflag && dfs_mount_defaults() != DFS_OK)
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

    if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (uint8_t *)path, DFS_DIRECTORY, dfs_scratch, fileinfo))
    {
        free(fileinfo);
        errno = EEXIST;
        return -1;
    }

    if (DFS_OK == DFS_OpenFile(&dfs_volinfo, (uint8_t *)path, DFS_DIRECTORY | DFS_WRITE, dfs_scratch, fileinfo))
    {
        int i;
        int secperclus = dfs_volinfo.secperclus;
        int firstcluster = fileinfo->firstcluster;
        int firstsector = dfs_volinfo.dataarea + ((firstcluster - 2) * secperclus);

        for (i = 0; i < secperclus; i++)
        {
            if (i < 2)
                memset(dfs_scratch, 0, sizeof(dfs_scratch));
            if (i == 0)
            {
                PDIRENT pde = (PDIRENT)dfs_scratch;
                strcpy((char*)pde->name, ".          ");
                DFS_SetStartCluster(pde, fileinfo->firstcluster);
                pde->attr = DFS_DIRECTORY;
                DFS_SetDirEntDateTime(pde);

                PDIRENT pde1 = pde + 1;
                memcpy(pde1, pde, sizeof(DIRENT));
                pde1->name[1] = '.';
                DFS_SetStartCluster(pde1, fileinfo->parentcluster);
            }

            DFS_WriteSector(0, dfs_scratch, firstsector++, 1);
        }
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
