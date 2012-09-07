/*
# #########################################################
# This file contains the chdir and getcwd functions
#   
# Written by Dave Hein
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
#include <unistd.h>
#include "../drivers/sd_internal.h"

int dfs_stdio_errno(int errnum);
void dfs_resolve_path(const char *fname, char *path);

int chdir(const char *path1)
{
    int len;
    int retval;
    char path[MAX_PATH];
    DIRINFO dirinfo;

    if (!dfs_mountflag && dfs_mount_defaults() != DFS_OK)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path(path1, path);

    dirinfo.scratch = dfs_scratch;

    len = strlen(path);
    if (len > 1 && path[len-1] == '/') path[len-1] = 0;

    // Make sure it's a valid directory
    if (DFS_OK == (retval = DFS_OpenDir(&dfs_volinfo, (uint8_t *)path, &dirinfo)))
    {
        strcpy(dfs_currdir, path);
        return DFS_OK;
    }

    errno = dfs_stdio_errno(retval);

    return -1;
}

char *getcwd(char *buf, int size)
{
    int len = strlen(dfs_currdir) + 1;

    if (size < len)
    {
        errno = ERANGE;
        return 0;
    }

    memcpy(buf, dfs_currdir, len);
    return buf;
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
