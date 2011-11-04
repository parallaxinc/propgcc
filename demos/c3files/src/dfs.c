/*
# #########################################################
# This file contains the interface code for the DOSFS FAT
# file sytem driver.  The code is separated into three main
# sections, which consist of the standard file I/O driver,
# the basic file routines that call the DOSFS fucntions,
# and the low level sector read and write routines that are
# called by DOSFS.
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

#ifdef __PROPELLER_LMM__
#define FILE_CACHE_SIZE 32
#else
#define FILE_CACHE_SIZE 512
#endif

static VOLINFO volinfo;
static int mountflag = 0;
static __attribute__((section(".hub"))) uint8_t scratch[512];

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
    NULL,  /* seek; not applicable */
    NULL,  /* remove; not applicable */
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

//#####################################
// DOSFS interface routines
//#####################################
static void error(char *str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}

static void warning(char *str)
{
    fprintf(stderr, "%s\n", str);
}

uint32_t dfs_read(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num)
{
    int retval;
    uint32_t successcount;

    retval = DFS_ReadFile(fileinfo, scratch, buffer, &successcount, num);
    if (retval != DFS_OK) successcount = 0;
    return successcount;
}

uint32_t dfs_write(PFILEINFO fileinfo, uint8_t *buffer, uint32_t num)
{
    int retval;
    uint32_t successcount;

    retval = DFS_WriteFile(fileinfo, scratch, buffer, &successcount, num);
    if (retval != DFS_OK) successcount = 0;
    return successcount;
}

uint32_t dfs_mount()
{
    int retval;

    if (mountflag) return 0;

    InitFileIO();
    MountFS();
    retval = DFS_OK;
    mountflag = (retval == DFS_OK); 
    return retval;
}

PFILEINFO dfs_open(const char *fname, const char *mode)
{
    int retval;
    int modeflag;
    PFILEINFO fileinfo;

    if (!mountflag)
        return 0;

#ifdef FILE_CACHE_SIZE
    fileinfo = malloc(sizeof(FILEINFO) + FILE_CACHE_SIZE);
#else
    fileinfo = malloc(sizeof(FILEINFO));
#endif

    if (!fileinfo)
        return 0;

    if (mode[0] == 'w' && mode[1] != 'a')
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_READ, scratch, fileinfo);
        if (retval == DFS_OK)
            DFS_UnlinkFile(&volinfo, (char *)fname, scratch);
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_WRITE | DFS_READ, scratch, fileinfo);
    }
    else if (mode[0] == 'w' && mode[1] == 'a')
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_WRITE | DFS_READ, scratch, fileinfo);
        if (retval == DFS_OK)
            DFS_Seek(fileinfo, 0xffffffff, scratch);
    }
    else
    {
        retval = DFS_OpenFile(&volinfo, (char *)fname, DFS_READ, scratch, fileinfo);
    }

    if (retval != DFS_OK)
    {
        free(fileinfo);
        fileinfo = 0;
    }

    return fileinfo;
}

int dfs_close(PFILEINFO fileinfo)
{
    free(fileinfo);
    return 0;
}

PDIRINFO dfs_opendir(char *dirname)
{
    int i;
    int retval;
    PDIRINFO dirinfo = malloc(sizeof(DIRINFO) + sizeof(DIRENT) + SECTOR_SIZE);

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

int dfs_remove(char *fname)
{
    return DFS_UnlinkFile(&volinfo, fname, scratch);
}

//#####################################
// Low level routines for initialization
// and reading and writing sectors.
//#####################################
#undef  FS_DEBUG

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

volatile uint32_t *xmm_mbox;

#ifdef __PROPELLER_LMM__
static int32_t XMM_MBOX[2];
static int32_t CACHE_ADDR[8];
#endif

static void LoadCacheDriver(void)
{
#ifndef __PROPELLER_LMM__
    extern uint16_t _xmm_mbox_p;
    xmm_mbox = (uint32_t *)(uint32_t)_xmm_mbox_p;
#else
    extern uint8_t c3_cache_array[];
    uint32_t params[4];
    params[0] = (int32_t)XMM_MBOX;
    params[1] = (int32_t)CACHE_ADDR;
    params[2] = 2;
    params[3] = 2;
    cognew(c3_cache_array, params);
    xmm_mbox = (uint32_t *)XMM_MBOX;
    xmm_mbox[0] = 1;
    while (xmm_mbox[0]);
#endif
}

int MountFS(void)
{
    int i;
    uint32_t start, err;

    // Find the first sector of the volume
    DFS_ReadSector(0, scratch, 0, 1);
    if (!strncmp(scratch+0x36, "FAT16", 5) ||
        !strncmp(scratch+0x52, "FAT32", 5))
        start = 0;
    else
        start = scratch[0x1c6] | (scratch[0x1c7] << 8) |
                (scratch[0x1c8] << 16) | (scratch[0x1c9] << 24);

#ifdef FS_DEBUG
    printf("GetVolInfo:\r\n");
#endif
    err = DFS_GetVolInfo(0, scratch, start, &volinfo);
#ifdef FS_DEBUG
    printf(" -->%ld\r\n", err);
#endif
    if (err != DFS_OK)
        return FALSE;

#ifdef FS_DEBUG
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
#endif

    return TRUE;
}

#define SD_INIT_CMD             0x0d
#define SD_READ_CMD             0x11
#define SD_WRITE_CMD            0x15

static uint32_t __attribute__((section(".hubtext"))) do_cmd(uint32_t cmd)
{
    xmm_mbox[0] = cmd;
    while (xmm_mbox[0]);
    return xmm_mbox[1];
}

int InitFileIO(int retries)
{
    uint32_t result;
    LoadCacheDriver();
    while (retries == 0 || --retries >= 0) {
        result = do_cmd(SD_INIT_CMD);
        if (result == 0)
            return 0;
        printf("Retrying SD init: %d\n", result);
    }
    return -1;
}

uint32_t DFS_ReadSector(uint8_t unit, uint8_t *buffer, uint32_t sector, uint32_t count)
{
    uint32_t params[3], result;
    while (count > 0) {
        if (((uint32_t)buffer) & 0xffff0000)
            params[0] = (uint32_t)scratch;
        else
            params[0] = (uint32_t)buffer;
        params[1] = SECTOR_SIZE;
        params[2] = sector;
        result = do_cmd(SD_READ_CMD | ((uint32_t)params << 8));
        if (result != 0) {
            printf("SD_READ_CMD failed: %d\n", result);
            return -1;
        }
        if (((uint32_t)buffer) & 0xffff0000)
            memcpy(buffer, scratch, SECTOR_SIZE);
        buffer += SECTOR_SIZE;
        ++sector;
        --count;
    }

    return 0;
}

uint32_t DFS_WriteSector(uint8_t unit, uint8_t *buffer, uint32_t sector, uint32_t count)
{
    uint32_t params[3], result;
    while (count > 0) {
        if (((uint32_t)buffer) & 0xffff0000)
        {
            memcpy(scratch, buffer, SECTOR_SIZE);
            params[0] = (uint32_t)scratch;
        }
        else
            params[0] = (uint32_t)buffer;
        params[1] = SECTOR_SIZE;
        params[2] = sector;
        result = do_cmd(SD_WRITE_CMD | ((uint32_t)params << 8));
        if (result != 0) {
            printf("SD_WRITE_CMD failed: %d\n", result);
            return -1;
        }
        buffer += SECTOR_SIZE;
        ++sector;
        --count;
    }
    return 0;
}

/*
+--------------------------------------------------------------------
¦  TERMS OF USE: MIT License
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
