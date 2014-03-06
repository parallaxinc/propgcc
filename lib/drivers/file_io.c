/*
# #########################################################
# This file contains the SD driver code for the C3
# Propeller board.  It contains the low-level three file
# system routines, which are DFS_InitFileIO, DFS_ReadSector
# and DFS_WriteSector.
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
#include "sd_internal.h"

#define SD_INIT_CMD  0x0d
#define SD_READ_CMD  0x11
#define SD_WRITE_CMD 0x15
#define BUS_LOCK_CMD 0x1D

_NATIVE extern void _enable_spi_locking(int lock_id);
static volatile uint32_t __attribute__((section(".hub"))) sd_lock = -1;
static volatile uint32_t __attribute__((section(".hub"))) *sd_mbox;

#ifdef __PROPELLER_USE_XMM__
#define USE_XMM_MBOX
extern uint16_t _xmm_mbox_p;
#endif
uint32_t *_sd_mbox_p = 0;

// This routine sends a command to the driver cog and returns the result
// It must be loaded in HUB RAM so we don't generate a cache miss
static uint32_t __attribute__((section(".hubtext"))) do_cmd(uint32_t cmd)
{
    sd_mbox[0] = cmd;
    while (sd_mbox[0]);
    return sd_mbox[1];
}

// This routine passes a bus lock to the low-level driver
void dfs_use_lock(uint32_t lockId)
{
#ifdef __PROPELLER_USE_XMM__
    if (sd_mbox)
        _enable_spi_locking(lockId);
    else
#endif
        sd_lock = lockId;
}

// This routine initializes the low-level driver
int DFS_InitFileIO(void)
{
    int retries = 5;

#if !defined(USE_XMM_MBOX)
    sd_mbox = _sd_mbox_p;
    if (!_sd_mbox_p)
        return DFS_INVAL;
#else
    if (_sd_mbox_p)
        sd_mbox = _sd_mbox_p;
    else
    {
        sd_mbox = (uint32_t *)(uint32_t)_xmm_mbox_p;
        retries = -1;    // We're using the SD Cache driver - it's already initialized!
    }
#endif

    // Handle the case where we were given the lock before dfs_mount() was called.
    if (sd_lock != -1)
        dfs_use_lock(sd_lock);

    while (retries-- > 0)
    {
        if (do_cmd(SD_INIT_CMD) == 0)
            return DFS_OK;
    }

    return DFS_IOERR;
}

// This routine reads 512-byte sectors into a buffer.  If the buffer is
// not located in hub memory it reads the sectors into the scratch buffer,
// and then copies it to to the buffer.
uint32_t DFS_ReadSector(uint8_t unit, uint8_t *buffer, uint32_t sector, uint32_t count)
{
    uint32_t params[3], result;
    while (count > 0) {
        if (((uint32_t)buffer) & 0xffff0000)
            params[0] = (uint32_t)dfs_scratch;
        else
            params[0] = (uint32_t)buffer;
        params[1] = SECTOR_SIZE;
        params[2] = sector;
        result = do_cmd(SD_READ_CMD | ((uint32_t)params << 8));
        if (result != 0) {
            return DFS_IOERR;
        }
        if (((uint32_t)buffer) & 0xffff0000)
            memcpy(buffer, dfs_scratch, SECTOR_SIZE);
        buffer += SECTOR_SIZE;
        ++sector;
        --count;
    }

    return DFS_OK;
}

// This routine writes 512-byte sectors from a buffer.  If the buffer is
// not located in hub memory the data is first copied to the scratch buffer,
// and then written from the scratch buffer.
uint32_t DFS_WriteSector(uint8_t unit, uint8_t *buffer, uint32_t sector, uint32_t count)
{
    uint32_t params[3], result;
    while (count > 0) {
        if (((uint32_t)buffer) & 0xffff0000)
        {
            memcpy(dfs_scratch, buffer, SECTOR_SIZE);
            params[0] = (uint32_t)dfs_scratch;
        }
        else
            params[0] = (uint32_t)buffer;
        params[1] = SECTOR_SIZE;
        params[2] = sector;
        result = do_cmd(SD_WRITE_CMD | ((uint32_t)params << 8));
        if (result != 0) {
            return DFS_IOERR;
        }
        buffer += SECTOR_SIZE;
        ++sector;
        --count;
    }
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
