/*
# #########################################################
# This file contains the standard file I/O driver that is
# uses the DOSFS FAT file I/O routines.
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
#include "dosfs.h"

#ifndef __PROPELLER_USE_XMM__
#define FILE_BUFFER_SIZE 16     // Use a small file buffer for CMM/LMM
#else
#define FILE_BUFFER_SIZE 512    // Use a 512-byte file buffer for XMM/XMMC
#endif

// Global variables
VOLINFO dfs_volinfo;
int dfs_mountflag = 0;
char dfs_currdir[MAX_PATH];
__attribute__((section(".hub"))) uint8_t dfs_scratch[512];

// Function prototypes
extern void LoadSDDriver(uint32_t configwords[2]);
int DFS_InitFileIO(void);
int dfs_stdio_errno(int errnum);
void dfs_resolve_path(const char *fname, char *path);
uint32_t mount_complete(void);

// Define the the file prefix
static const char File_prefix[] = "";

// function called by fopen
static int File_fopen(FILE *fp, const char *fname1, const char *mode)
{
    int retval;
    PFILEINFO fileinfo;
    char fname[MAX_PATH];
    uint8_t dfs_mode;

    if (!dfs_mountflag && dfs_mount_defaults() != DFS_OK)
    {
        errno = EIO;
        return -1;
    }

    dfs_resolve_path((char *)fname1, fname);

    fileinfo = malloc(sizeof(FILEINFO) + FILE_BUFFER_SIZE);

    if (!fileinfo)
    {
        errno = ENOMEM;
        return -1;
    }

    if (mode[1] == '+')
        dfs_mode = DFS_READ | DFS_WRITE;
    else if (mode[0] == 'r')
        dfs_mode = DFS_READ;
    else
        dfs_mode = DFS_WRITE;

    if (mode[0] == 'w')
    {
        retval = DFS_OpenFile(&dfs_volinfo, (uint8_t *)fname, DFS_READ, dfs_scratch, fileinfo);
        if (retval == DFS_OK)
            DFS_UnlinkFile(&dfs_volinfo, (uint8_t *)fname, dfs_scratch);
        retval = DFS_OpenFile(&dfs_volinfo, (uint8_t *)fname, dfs_mode, dfs_scratch, fileinfo);
    }
    else if (mode[0] == 'a')
    {
        retval = DFS_OpenFile(&dfs_volinfo, (uint8_t *)fname, dfs_mode, dfs_scratch, fileinfo);
    }
    else if (mode[0] == 'r')
    {
        retval = DFS_OpenFile(&dfs_volinfo, (uint8_t *)fname, DFS_READ, dfs_scratch, fileinfo);
        if (retval == DFS_OK && (dfs_mode & DFS_WRITE))
            retval = DFS_OpenFile(&dfs_volinfo, (uint8_t *)fname, dfs_mode, dfs_scratch, fileinfo);
    }
    else
    {
        errno = EINVAL;
        free(fileinfo);
        return -1;
    }

    if (retval != DFS_OK)
    {
        errno = dfs_stdio_errno(retval);
        free(fileinfo);
        return -1;
    }

    fp->drvarg[0] = (int)fileinfo;

    // Set up file buffer in the FILE struct
    fp->_ptr = fp->_base = ((void *)fileinfo) + sizeof(FILEINFO);
    fp->_bsiz = FILE_BUFFER_SIZE;
    fp->_flag |= _IOFREEBUF;

    return 0;
}

// function called by fclose
static int File_fclose(FILE *fp)
{
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    free(dfs_fp);
    return 0;
}

// function called by fread
static int File_read(FILE *fp, unsigned char *buf, int count)
{
    int retval;
    uint32_t successcount;
    PFILEINFO fileinfo = (PFILEINFO)fp->drvarg[0];

    retval = DFS_ReadFile(fileinfo, dfs_scratch, buf, &successcount, count);
    if (retval != DFS_OK)
    {
        successcount = 0;
        errno = dfs_stdio_errno(retval);
    }
    return successcount;
}

// function called by fwrite
static int File_write(FILE *fp, unsigned char *buf, int count)
{
    int retval;
    uint32_t successcount;
    PFILEINFO fileinfo = (PFILEINFO)fp->drvarg[0];

    retval = DFS_WriteFile(fileinfo, dfs_scratch, buf, &successcount, count);
    if (retval != DFS_OK)
    {
        successcount = 0;
        errno = dfs_stdio_errno(retval);
    }
    return successcount;
}

// function called by fseek
static int File_fseek(FILE *fp, long int offset, int origin)
{
    // This code needs to be enabled and tested
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    if (origin == SEEK_CUR)
        offset += dfs_fp->pointer;
    else if (origin == SEEK_END)
        offset += dfs_fp->filelen;
    else if (origin != SEEK_SET)
    {
        errno = EIO;
        return -1;
    }
    if (offset < 0)
        offset = 0;
    DFS_Seek(dfs_fp, offset, dfs_scratch);
    return 0;
}

// function called by remove
static int File_remove(const char *fname)
{
    int errnum;
    int retval = -1;
    //DIRINFO dirinfo;
    char path[MAX_PATH];

    dfs_resolve_path(fname, path);

    if (!(errnum = DFS_UnlinkFile(&dfs_volinfo, (uint8_t *)path, dfs_scratch)))
        retval = 0;
    // OpenDir seems to be causing a problem, so it's commented out for now
    //else if (!DFS_OpenDir(&dfs_volinfo, (uint8_t *)path, &dirinfo))
    //    errno = EISDIR;
    else
        errno = dfs_stdio_errno(errnum);
    
    return retval;
}

// Define the driver list
_Driver _FileDriver =
{
    File_prefix,
    File_fopen,
    File_fclose,
    File_read,
    File_write,
    File_fseek,
    File_remove,
};

// Convert the dosfs errno to the stdio errno 
int dfs_stdio_errno(int errnum)
{
    switch(errnum)
    {
        case DFS_OK:
            break;
        case DFS_EOF:
            errnum = EOK;
            break;
        case DFS_WRITEPROT:
            errnum = EROFS;
            break;
        case DFS_NOTFOUND:
            errnum = ENOENT;
            break;
        case DFS_PATHLEN:
            errnum = ENAMETOOLONG;
            break;
        default:
            errnum = EIO;
    }
    return errnum;
}

/* these variables are patched by the loader */
int _cfg_sdspi_config1 = -1;
int _cfg_sdspi_config2 = -1;

uint32_t dfs_mount_defaults(void)
{
    uint32_t configwords[2] = { _cfg_sdspi_config1, _cfg_sdspi_config2 };
    LoadSDDriver(configwords);
    return mount_complete();
}

uint32_t mount_complete(void)
{
    int retval, sector;

    strcpy(dfs_currdir, "/");

    // Start up the low-level file I/O driver
    if ((retval = DFS_InitFileIO()) != DFS_OK)
        return retval;

    // Find the first sector of the volume
    if ((retval = DFS_ReadSector(0, dfs_scratch, 0, 1)) != DFS_OK)
        return retval;
    if (!strncmp((char *)dfs_scratch+0x36, "FAT16", 5) ||
        !strncmp((char *)dfs_scratch+0x52, "FAT32", 5))
        sector = 0;
    else
        memcpy(&sector, &dfs_scratch[0x1c6], 4);

    // Get the volume information
    retval = DFS_GetVolInfo(0, dfs_scratch, sector, &dfs_volinfo);
    dfs_mountflag = (retval == DFS_OK); 

    return retval;
}

static void movestr(char *ptr1, char *ptr2)
{
    while (*ptr2) *ptr1++ = *ptr2++;
    *ptr1 = 0;
}

void dfs_resolve_path(const char *fname, char *path)
{
    char *ptr;
    char *ptr1;

    if (!strcmp(fname, ".")) fname++;
    else if (!strncmp(fname, "./",2) || !strncmp(fname, ".\\",2)) fname += 2;

    if (fname[0] == '/' || fname[0] == '\\')
        strcpy(path, fname);
    else if (fname[0] == 0)
        strcpy(path, dfs_currdir);
    else
    {
        strcpy(path, dfs_currdir);
        int pathLen = strlen(path);
        if (path[pathLen-1] != '/' && path[pathLen-1] != '\\') strcat(path, "/");
        strcat(path, fname);
    }

    // Whack DOS paths to UNIX paths
    for (ptr = path;  *ptr;  ptr++)
        if (*ptr == '\\')
            *ptr = '/';

    // Process ..
    ptr = path;
    while (*ptr)
    {
        if (!strncmp(ptr, "/..", 3) && (ptr[3] == 0 || ptr[3] == '/'))
        {
            if (ptr == path)
            {
               movestr(ptr, ptr+3);
            }
            else
            {
                ptr1 = ptr - 1;
                while (ptr1 != path)
                {
                    if (*ptr1 == '/') break;
                    ptr1--;
                }
                movestr(ptr1, ptr+3);
                ptr = ptr1;
            }
        }
        else
        {
            ptr++;
            while (*ptr)
            {
               if (*ptr == '/') break;
               ptr++;
            }
        }
    }

    if (path[0] == 0) strcpy(path, "/");
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
