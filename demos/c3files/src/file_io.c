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
#include "dosfs.h"
#include "dfs.h"

#ifdef SPINNERET
#define SD_DO  16
#define SD_CLK 21
#define SD_DI  20
#define SD_CS  19
#endif

#define SD_INIT_CMD  0x0d
#define SD_READ_CMD  0x11
#define SD_WRITE_CMD 0x15

extern __attribute__((section(".hub"))) uint8_t dfs_scratch[512];

#ifdef __PROPELLER_LMM__
static volatile uint32_t xmm_mbox[2];

// This routine starts the SD driver cog
static void LoadSDDriver(uint8_t *pins)
{
    int32_t pinmask[5];
    extern uint8_t sd_driver_array[];

    // Overwrite pin masks if pins are specified
    if (pins)
    {
        pinmask[0] = 1 << pins[0];  // SD MISO
        pinmask[1] = 1 << pins[1];  // SD CLK
        pinmask[2] = 1 << pins[2];  // SD MOSI
        pinmask[3] = 1 << pins[3];  // SD CS
        pinmask[4] = 0;             // Zero disables C3 mode
        memcpy(&sd_driver_array[4], pinmask, 20);
    }

    xmm_mbox[0] = 1;
    cognew(sd_driver_array, (uint32_t *)xmm_mbox);
    while (xmm_mbox[0]);
}
#else
static volatile uint32_t *xmm_mbox;

// This routine is used in XMM/XMMC mode where
// the SD driver is already started
static void LoadSDDriver(uint8_t *pins)
{
    extern uint16_t _xmm_mbox_p;
    xmm_mbox = (uint32_t *)(uint32_t)_xmm_mbox_p;
}
#endif

// This routine sends a command to the driver cog and returns the result
// It must be loaded in HUB RAM so we don't generate a cache miss
static uint32_t __attribute__((section(".hubtext"))) do_cmd(uint32_t cmd)
{
    xmm_mbox[0] = cmd;
    while (xmm_mbox[0]);
    return xmm_mbox[1];
}

// This routine initializes the low-level driver
int DFS_InitFileIO(uint8_t *parms)
{
    int retries = 5;
    uint32_t result;
    LoadSDDriver(parms);
    while (retries-- > 0)
    {
        result = do_cmd(SD_INIT_CMD);
        if (result == 0)
            return DFS_OK;
        printf("Retrying SD init: %d\n", result);
    }
    return -1;
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
            printf("SD_READ_CMD failed: %d\n", result);
            return -1;
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
            printf("SD_WRITE_CMD failed: %d\n", result);
            return -1;
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
