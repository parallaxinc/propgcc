#include <stdio.h>
#include "sdio.h"
#include "debug.h"

#define SECTOR_SIZE     512

#define SD_INIT_CMD     0x0d
#define SD_READ_CMD     0x11
#define SD_WRITE_CMD    0x15

volatile uint32_t __attribute__((section(".hub"))) *sd_mbox;

static uint32_t __attribute__((section(".hubtext"), noinline)) do_cmd(uint32_t cmd)
{
    sd_mbox[0] = cmd;
    while (sd_mbox[0])
        ;
    return sd_mbox[1];
}

int SD_Init(volatile uint32_t *mbox, int retries)
{
    // remember the mailbox address
    sd_mbox = mbox;

    // initialize the SD card interface
    while (--retries >= 0) {
        uint32_t result;
        result = do_cmd(SD_INIT_CMD);
        if (result == 0)
            return 0;
        DPRINTF("Retrying SD init: %d\n", result);
    }
    
    // initialization failed
    return -1;
}

int SD_ReadSector(uint32_t sector, void *buffer)
{
    uint32_t params[3], result;
    params[0] = (uint32_t)buffer;
    params[1] = SECTOR_SIZE;
    params[2] = sector;
    result = do_cmd(SD_READ_CMD | ((uint32_t)params << 8));
    if (result != 0) {
        DPRINTF("SD_READ_CMD failed: %d\n", result);
        return -1;
    }
    return 0;
}
