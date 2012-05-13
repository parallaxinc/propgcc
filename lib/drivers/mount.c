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
#include <errno.h>
#include <propeller.h>
#include "sd_internal.h"

/* BUG: these need to be moved to a common file that can be shared by the loader */
#define  CS_CLR_PIN_MASK       0x01   // either CS or CLR
#define  INC_PIN_MASK          0x02   // for C3-style CS
#define  MUX_START_BIT_MASK    0x04   // low order bit of mux field
#define  MUX_WIDTH_MASK        0x08   // width of mux field
#define  ADDR_MASK             0x10   // device number for C3-style CS or value to write to the mux

extern void LoadSDDriver(uint32_t configwords[2]);
extern uint32_t mount_complete(void);

uint32_t dfs_mount(_SD_Params* params)
{
    if (dfs_mountflag)
        return DFS_OK;

    if (params) {
        uint32_t* pins = (uint32_t*)&params->pins;
        uint32_t configwords[2];
        if (params->AttachmentType == _SDA_ConfigWords) {
            configwords[0] = params->pins.ConfigWords.CONFIG1;
            configwords[1] = params->pins.ConfigWords.CONFIG2;
        }
        else {
            memset(configwords, 0, sizeof(configwords));
            configwords[0] = (pins[2]<<24) | (pins[0]<<16) | (pins[1]<<8);
            
            switch (params->AttachmentType) {
            case _SDA_SingleSPI:
                configwords[0] |= CS_CLR_PIN_MASK;
                configwords[1] = params->pins.SingleSPI.CS<<24;
                break;
            case _SDA_SerialDeMUX:
                configwords[0] |= CS_CLR_PIN_MASK | INC_PIN_MASK | ADDR_MASK;
                configwords[1] = (params->pins.SerialDeMUX.CLR<<24) | (params->pins.SerialDeMUX.INC<<16) | params->pins.SerialDeMUX.ADDR;
                break;
            case _SDA_ParallelDeMUX:
                configwords[0] |= CS_CLR_PIN_MASK | MUX_START_BIT_MASK | MUX_WIDTH_MASK | ADDR_MASK;
                configwords[1] = (params->pins.ParallelDeMUX.CS<<24) | (params->pins.ParallelDeMUX.START<<16) | (params->pins.ParallelDeMUX.WIDTH<<8) | params->pins.ParallelDeMUX.ADDR;
                break;
            default:
                /* should never be reached */
                break;
            }
        }
        LoadSDDriver(configwords);
    }

    return mount_complete();
}
