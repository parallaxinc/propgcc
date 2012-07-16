/*
# #########################################################
# This file contains the interface code for the DOSFS FAT
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
#include <dirent.h>
#include "../drivers/sd_internal.h"

int dfs_stdio_errno(int errnum);
void dfs_resolve_path(const char *fname, char *path);

DIR *opendir(const char *dirname1)
{
    int retval;
    PDIRINFO dirinfo;
    char dirname[MAX_PATH];

    if (!dfs_mountflag && dfs_mount_defaults() != DFS_OK)
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
    retval = DFS_OpenDir(&dfs_volinfo, (uint8_t *)dirname, dirinfo);

    if (retval == DFS_OK)
        return (DIR *)dirinfo;

    free(dirinfo);

    errno = dfs_stdio_errno(retval);

    return 0;
}

static void ConvertFilename(char *str1, char *str2)
{
    int i;

    for (i = 0; i < 8; i++)
    {
        if (str1[i] == ' ') break;
        *str2++ = str1[i];
    }
    if (str1[8] != ' ')
    {
        *str2++ = '.';
        for (i = 8; i < 11; i++)
        {
            if (str1[i] == ' ') break;
            *str2++ = str1[i];
        }
    }
    *str2 = 0;
}

struct dirent *readdir(DIR *dirinfo)
{
    int retval;
    struct dirent *pdirent = ((void *)dirinfo) + sizeof(DIRINFO);

    while (1)
    {
        retval = DFS_GetNext(&dfs_volinfo, (PDIRINFO)dirinfo, (PDIRENT)pdirent);
        if (retval != DFS_OK) return 0;
        if (pdirent->name[0])
        {
            ConvertFilename((char *)pdirent->name, pdirent->d_name);
            break;
        }
    }

    return pdirent;
}

int closedir(DIR *dirinfo)
{
    if (!dirinfo)
    {
        errno = EIO;
        return -1;
    }

    free(dirinfo);

    return 0;
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
