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
#include "dosfs.h"
#include "dfs.h"

#undef  FS_DEBUG               // Print debug volume information
#define CURRENT_DIRECTORY      // Include support for the current directory
#define MAKE_REMOVE_DIRECTORY  // Include fds_mkdir and fds_rmdir

#ifdef __PROPELLER_LMM__
#define FILE_CACHE_SIZE 16     // Use a small file cache for LMM
#define CHECK_MEMORY_SPACE     // Check heap/malloc space for LMM
#else
#define FILE_CACHE_SIZE 512    // Use a 512-byte file cache for XMM/XMMC
#endif

static VOLINFO volinfo;
static int mountflag = 0;
__attribute__((section(".hub"))) uint8_t dfs_scratch[512];

#ifdef CURRENT_DIRECTORY
static char currdir[MAX_PATH];
static void ResolvePath(const char *fname, char *path);
#endif

//#####################################
// Device driver
//#####################################
extern _Driver _SimpleSerialDriver;

const char FilePrefix[] = "";

_Driver FileDriver =
  {
    FilePrefix,
    File_fopen,
    File_fclose,
    File_read,
    File_write,
    File_fseek,   /* TODO: test fseek */
    File_remove,  /* TODO: need to add remove to library */
  };

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */
_Driver *_driverlist[] = {
  &_SimpleSerialDriver,
  &FileDriver,
  NULL
};

/* function called by fopen */
int File_fopen(FILE *fp, const char *str, const char *mode)
{
    PFILEINFO dfs_fp = (PFILEINFO)dfs_open(str, mode);
    fp->drvarg[0] = (int)dfs_fp;
#ifdef FILE_CACHE_SIZE
    // Set up file cache in FILE struct
    if (dfs_fp)
    {
        fp->_ptr = fp->_base = ((void *)dfs_fp) + sizeof(FILEINFO);
        fp->_bsiz = FILE_CACHE_SIZE;
        fp->_flag |= _IOFREEBUF;
    }
#endif
    return (int)dfs_fp;
}

/* function called by fclose */
int File_fclose(FILE *fp)
{
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    return dfs_close(dfs_fp);
}

/* function called by fwrite */
int File_write(FILE *fp, unsigned char *buf, int count)
{
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    return dfs_write(dfs_fp, buf, count);
}

/* function called by fread */
int File_read(FILE *fp, unsigned char *buf, int count)
{
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    return dfs_read(dfs_fp, buf, count);
}

/* function called by fseek */
int File_fseek(FILE *fp, long int offset, int origin)
{
#if 0
    // This code needs to be enabled and tested
    PFILEINFO dfs_fp = (PFILEINFO)fp->drvarg[0];
    if (origin == SEEK_CUR)
        offset += dfs_fp->pointer;
    else if (origin == SEEK_END)
        offset += dfs_fp->filelen;
    else if (origin != SEEK_SET)
        return -1;
    if (offset < 0 || offset > dfs_fp->filelen)
        return -1;
    DFS_Seek(dfs_fp, offset, dfs_scratch);
#endif
    return 0;
}

/* function called by remove */
int File_remove(const char *str)
{
    return 0;
}

//#####################################
// DOSFS interface routines
//#####################################
#ifdef CHECK_MEMORY_SPACE
#define MALLOC dfs_malloc  // Map MALLOC to local debug routine

void *dfs_malloc(int size)
{
    int space;
    void *ptr = malloc(size);
    static int prevspace = 100000;

    if (ptr)
    {
        space = ((int)&space) - size - (int)ptr;
        if (space < prevspace)
        {
            prevspace = space;
            if (space < 700)
                printf("WARNING: Heap/Stack space = %d\n", space);
        }
    }
    else
        printf("malloc failed\n");

    return ptr;
}
#else
#define MALLOC malloc
#endif

void dfs_perror(int errnum, char *str)
{
    if (str)
        printf("%s ", str);
    if (errnum == DFS_MALLOC_FAILED)
        printf("malloc failed\n");
    else if (errnum == DFS_NOT_A_DIRECTORY)
        printf("not a directory\n");
    else if (errnum == DFS_DOES_NOT_EXIST)
        printf("does not exist\n");
    else if (errnum == DFS_DIRECTORY_NOT_EMPTY)
        printf("directory not empty\n");
    else if (errnum == DFS_ALREADY_EXISTS)
        printf("already exists\n");
    else if (errnum == DFS_CREATE_FAILED)
        printf("create failed\n");
    else if (errnum == DFS_IS_A_DIRECTORY)
        printf("is a directory\n");
    else
        printf("error %d\n", errnum);
}

uint32_t dfs_read(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num)
{
    int retval;
    uint32_t successcount;

    retval = DFS_ReadFile(fileinfo, dfs_scratch, buffer, &successcount, num);
    if (retval != DFS_OK) successcount = 0;
    return successcount;
}

uint32_t dfs_write(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num)
{
    int retval;
    uint32_t successcount;

    retval = DFS_WriteFile(fileinfo, dfs_scratch, buffer, &successcount, num);
    if (retval != DFS_OK) successcount = 0;
    return successcount;
}

uint32_t dfs_mount(uint8_t *parms)
{
    int retval, start;

    if (mountflag) return 0;

#ifdef CURRENT_DIRECTORY
    strcpy(currdir, "/");
#endif

    // Start up the low-level file I/O driver
    DFS_InitFileIO(parms);

    // Find the first sector of the volume
    DFS_ReadSector(0, dfs_scratch, 0, 1);
    if (!strncmp(dfs_scratch+0x36, "FAT16", 5) ||
        !strncmp(dfs_scratch+0x52, "FAT32", 5))
        start = 0;
    else
        start = dfs_scratch[0x1c6] | (dfs_scratch[0x1c7] << 8) |
                (dfs_scratch[0x1c8] << 16) | (dfs_scratch[0x1c9] << 24);

    // Get the volume information
    retval = DFS_GetVolInfo(0, dfs_scratch, start, &volinfo);
    mountflag = (retval == DFS_OK); 

#ifdef FS_DEBUG
    // Print volume information
    MountFSDebug();
#endif

    return retval;
}

PFILEINFO dfs_open(const char *fname1, const char *mode)
{
    int retval;
    int modeflag;
    PFILEINFO fileinfo;
#ifdef CURRENT_DIRECTORY
    char fname[MAX_PATH];
    ResolvePath((char *)fname1, fname);
#else
    char *fname = fname1;
#endif

    if (!mountflag)
        return 0;

#ifdef FILE_CACHE_SIZE
    fileinfo = MALLOC(sizeof(FILEINFO) + FILE_CACHE_SIZE);
#else
    fileinfo = MALLOC(sizeof(FILEINFO));
#endif

    if (!fileinfo)
        return 0;

    if (mode[0] == 'w' && mode[1] != 'a')
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_READ, dfs_scratch, fileinfo);
        if (retval == DFS_OK)
            DFS_UnlinkFile(&volinfo, (char *)fname, dfs_scratch);
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_WRITE | DFS_READ, dfs_scratch, fileinfo);
    }
    else if (mode[0] == 'w' && mode[1] == 'a')
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_WRITE | DFS_READ, dfs_scratch, fileinfo);
        if (retval == DFS_OK)
            DFS_Seek(fileinfo, 0xffffffff, dfs_scratch);
    }
    else
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_READ, dfs_scratch, fileinfo);
    }

    if (retval != DFS_OK)
    {
        free(fileinfo);
        fileinfo = 0;
    }

    return fileinfo;
}

static int dfs_setattr(const char *path, int attr)
{
    uint8_t *ptr;
    PFILEINFO fileinfo;
    int dirsector, diroffset;

    if (!mountflag)
        return -3;

    fileinfo = MALLOC(sizeof(FILEINFO));

    if (!fileinfo)
        return -2;

    if (DFS_OK != DFS_OpenFile(&volinfo, (char *)path, DFS_DIRECTORY, dfs_scratch, fileinfo))
    {
        free(fileinfo);
        return -1;
    }

    dirsector = fileinfo->dirsector;
    diroffset = fileinfo->diroffset;
    ptr = &dfs_scratch[diroffset*32];

    DFS_ReadSector(0, dfs_scratch, dirsector, 1);
    if (attr >= 0)
    {
        ptr[11] = attr;
        DFS_WriteSector(0, dfs_scratch, dirsector, 1);
    }
    else
        attr = ptr[11];

    free(fileinfo);

    return attr;
}

#ifdef MAKE_REMOVE_DIRECTORY
int dfs_rmdir(const char *path1)
{
    uint8_t *ptr;
    DIRENT dirent;
    DIRINFO dirinfo;
    FILEINFO fileinfo;
    int dirsector, diroffset;
#ifdef CURRENT_DIRECTORY
    char path[MAX_PATH];
    ResolvePath((char *)path1, path);
#else
    char *path = path1;
#endif

    dirinfo.scratch = dfs_scratch;

    // Open the directory
    if (DFS_OK != DFS_OpenDir(&volinfo, path, &dirinfo))
    {
        // Try opening as a file
        if (DFS_OK == DFS_OpenFile(&volinfo, (char *)path, DFS_READ, dfs_scratch, &fileinfo))
            return DFS_NOT_A_DIRECTORY;
        else
            return DFS_DOES_NOT_EXIST;
    }

    // Check if directory is empty
    while (DFS_OK == DFS_GetNext(&volinfo, &dirinfo, &dirent))
    {
        if (dirent.name[0])
            return DFS_DIRECTORY_NOT_EMPTY;
    }

    // Open the directory as a file
    if (DFS_OK != DFS_OpenFile(&volinfo, (char *)path, DFS_DIRECTORY, dfs_scratch, &fileinfo))
        return -1;

    // Remove the directory attribute so we can delete it
    dirsector = fileinfo.dirsector;
    diroffset = fileinfo.diroffset;
    ptr = &dfs_scratch[diroffset*32];
    DFS_ReadSector(0, dfs_scratch, dirsector, 1);
    ptr[11] = 0;
    DFS_WriteSector(0, dfs_scratch, dirsector, 1);

    // Delete the directory
    DFS_UnlinkFile(&volinfo, (char *)path, dfs_scratch);

    return DFS_OK;
}

int dfs_mkdir(const char *path1)
{
    PFILEINFO fileinfo;
#ifdef CURRENT_DIRECTORY
    char path[MAX_PATH];
    ResolvePath((char *)path1, path);
#else
    char *path = path1;
#endif

    if (!mountflag)
        return -1;

    fileinfo = MALLOC(sizeof(FILEINFO));

    if (!fileinfo)
        return -1;

    if (DFS_OK == DFS_OpenFile(&volinfo, (char *)path, DFS_DIRECTORY, dfs_scratch, fileinfo))
    {
        free(fileinfo);
        return DFS_ALREADY_EXISTS;
    }

    if (DFS_OK == DFS_OpenFile(&volinfo, (char *)path, DFS_DIRECTORY | DFS_WRITE, dfs_scratch, fileinfo))
    {
        int i;
        int secperclus = volinfo.secperclus;
        int firstcluster = fileinfo->firstcluster;
        int firstsector = volinfo.dataarea + ((firstcluster - 2) * secperclus);

        for (i = 0; i < 512; i += 32) dfs_scratch[i] = 0;
        for (i = 0; i < secperclus; i++)
            DFS_WriteSector(0, dfs_scratch, firstsector++, 1);
    }
    else
    {
        free(fileinfo);
        return DFS_CREATE_FAILED;
    }

    free(fileinfo);

    return DFS_OK;
}
#endif

int dfs_close(PFILEINFO fileinfo)
{
    free(fileinfo);
    return DFS_OK;
}

PDIRINFO dfs_opendir(char *dirname1)
{
    int i;
    int retval;
    PDIRINFO dirinfo = MALLOC(sizeof(DIRINFO) + sizeof(DIRENT) + SECTOR_SIZE);
#ifdef CURRENT_DIRECTORY
    char dirname[MAX_PATH];
    ResolvePath((char *)dirname1, dirname);
#else
    char *dirname = dirname1;
#endif

    if (!dirinfo)
        return 0;

    dirinfo->scratch = ((void *)dirinfo) + sizeof(DIRINFO) + sizeof(DIRENT);
    retval = DFS_OpenDir(&volinfo, dirname, dirinfo);

    if (retval == DFS_OK)
        return dirinfo;

    free(dirinfo);

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
        retval = DFS_GetNext(&volinfo, dirinfo, dirent);
        if (retval != DFS_OK) return 0;
        if (dirent->name[0]) break;
    }

    return dirent;
}

int dfs_remove(char *fname1)
{
#ifdef CURRENT_DIRECTORY
    char fname[MAX_PATH];
    ResolvePath(fname1, fname);
#else
    char *fname = fname1;
#endif
    int attr = dfs_setattr(fname, -1);
    if (attr < 0)
        return DFS_DOES_NOT_EXIST;
    if (attr & ATTR_DIRECTORY)
        return DFS_IS_A_DIRECTORY;
    return DFS_UnlinkFile(&volinfo, fname, dfs_scratch);
}

#ifdef FS_DEBUG
int MountFSDebug(void)
{
    printf("GetVolInfo:\r\n");

    printf("Volume label '%-11.11s'\r\n", volinfo.label);
    printf("%d sector/s per cluster, %d reserved sector/s, volume total %ld sectors.\r\n",
                volinfo.secperclus,
                volinfo.reservedsecs,
                volinfo.numsecs);
    printf("%ld sectors per FAT, first FAT at sector #%ld, root dir at #%ld.\r\n",
                volinfo.secperfat,
                volinfo.fat1,
                volinfo.rootdir);
    printf("(For FAT32, the root dir is a CLUSTER number, FAT12/16 it is a SECTOR number)\r\n");
    printf("%d root dir entries, data area commences at sector #%ld.\r\n",
                volinfo.rootentries,
                volinfo.dataarea);
    printf("%ld clusters (%ld bytes) in data area, filesystem IDd as ",
                volinfo.numclusters,
                volinfo.numclusters * volinfo.secperclus * SECTOR_SIZE);
    if (volinfo.filesystem == FAT12)
        printf("FAT12.\r\n");
    else if (volinfo.filesystem == FAT16)
        printf("FAT16.\r\n");
    else if (volinfo.filesystem == FAT32)
        printf("FAT32.\r\n");
    else {
        printf("[unknown]\r\n");
        return FALSE;
    }

    return TRUE;
}
#endif

#ifdef CURRENT_DIRECTORY
int dfs_chdir(const char *path1)
{
    int len;
    char path[MAX_PATH];
    DIRINFO dirinfo;
    ResolvePath(path1, path);

    dirinfo.scratch = dfs_scratch;

    len = strlen(path);
    if (len > 1 && path[len-1] == '/') path[len-1] = 0;

    // Make sure it's a valid directory
    if (DFS_OK == DFS_OpenDir(&volinfo, path, &dirinfo))
    {
        strcpy(currdir, path);
        return DFS_OK;
    }

    return -1;
}

char *dfs_getcd(void)
{
    return currdir;
}

static void movestr(char *ptr1, char *ptr2)
{
    while (*ptr2) *ptr1++ = *ptr2++;
    *ptr1 = 0;
}

static void ResolvePath(const char *fname, char *path)
{
    char *ptr;
    char *ptr1;

    if (!strcmp(fname, ".")) fname++;
    else if (!strncmp(fname, "./",2)) fname += 2;

    if (fname[0] == '/')
        strcpy(path, fname);
    else if (fname[0] == 0)
        strcpy(path, currdir);
    else
    {
        strcpy(path, currdir);
        if (path[strlen(path)-1] != '/') strcat(path, "/");
        strcat(path, fname);
    }

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
#endif

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
