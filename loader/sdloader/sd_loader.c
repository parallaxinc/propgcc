#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "fatread.h"
#include "sdio.h"
#include "../src/pex.h"
#include "sd_loader.h"

#define FILENAME    "AUTORUN PEX"

#define HUB_SIZE    (32 * 1024)

volatile uint32_t *xmm_mbox;
volatile uint32_t *sd_mbox;
volatile uint32_t *vm_mbox;
uint32_t cache_line_mask;

/* this section contains the info structure */
SdLoaderInfo __attribute__((section(".coguser0"))) info_data;
extern unsigned int _load_start_coguser0[];

/* this section contains the xmm_driver code */
uint32_t __attribute__((section(".coguser1"))) xmm_driver_data[496];
extern unsigned int _load_start_coguser1[];

/* this section contains the sd_driver code */
uint32_t __attribute__((section(".coguser2"))) sd_driver_data[496];
extern unsigned int _load_start_coguser2[];

/* this section contains the vm_start.S code */
extern unsigned int _load_start_coguser3[];

static void write_ram_long(uint32_t addr, uint32_t val);
static uint32_t erase_flash_block(uint32_t addr);
static uint32_t write_flash_page(uint32_t addr, uint8_t *buffer, uint32_t count);

int main(void)
{
    uint8_t *buffer = (uint8_t *)xmm_driver_data;
    SdLoaderInfo *info = (SdLoaderInfo *)_load_start_coguser0;
    uint32_t cache_addr;
    uint32_t params[4];
    VolumeInfo vinfo;
    FileInfo finfo;
    PexeFileHdr *hdr;
    uint32_t count;
    int sd_id, i;
    uint8_t *p;
    
    cache_addr = HUB_SIZE - info->cache_size;
    xmm_mbox = (volatile uint32_t *)(cache_addr - 2 * sizeof(uint32_t));
    vm_mbox = xmm_mbox - 1;
    
    if (info->use_cache_driver_for_sd) {
        sd_mbox = xmm_mbox;
        sd_id = -1;
    }
    else {
        sd_mbox = vm_mbox - 2;
        sd_mbox[0] = 0xffffffff;
        sd_id = cognew(_load_start_coguser2, sd_mbox);
        while (sd_mbox[0])
            ;
    }
    
    params[0] = (uint32_t)xmm_mbox;
    params[1] = cache_addr;
    params[2] = info->cache_param1;
    params[3] = info->cache_param2;
    
    // load the cache driver
    xmm_mbox[0] = 0xffffffff;
    cognew(_load_start_coguser1, params);
    while (xmm_mbox[0])
        ;
    cache_line_mask = params[0];
        
    if (SD_Init(sd_mbox, 5) != 0) {
        printf("SD card initialization failed\n");
        return -1;
    }
        
    if (MountFS(buffer, 5, &vinfo) != 0) {
        printf("MountFS failed\n");
        return 1;
    }
    
    // open the .pex file
    if (FindFile(buffer, &vinfo, FILENAME, &finfo) != 0) {
        printf("FindFile '%s' failed\n", FILENAME);
        return 1;
    }
    
    // read the .kernel cog image
    for (i = 0, p = buffer; i < 4; ++i) {
        if (GetNextFileSector(&finfo, p, &count) != 0) {
            printf("GetNextFileSector %d failed\n", i);
            return 1;
        }
        if (count != SECTOR_SIZE) {
            printf("Incomplete file header\n");
            return 1;
        }
        p += SECTOR_SIZE;
    }
    
    // start the xmm kernel
    hdr = (PexeFileHdr *)buffer;
    vm_mbox[0] = 0;
    cognew(hdr->kernel, vm_mbox);
    
    // setup the parameters for vm_start.S
    params[0] = hdr->loadAddress;
    params[1] = (uint32_t)xmm_mbox;
    params[2] = cache_line_mask;
    params[3] = (uint32_t)vm_mbox;

    // load into flash/eeprom
    if (hdr->loadAddress >= 0x30000000) {
        uint32_t addr = 0x00000000;
        printf("loading flash/eeprom\n");
        while (GetNextFileSector(&finfo, buffer, &count) == 0) {
            if ((addr & 0x00000fff) == 0)
                erase_flash_block(addr);
            write_flash_page(addr, buffer, count);
            addr += SECTOR_SIZE;
        }
    }
    
    // load into ram
    else {
        uint32_t addr = hdr->loadAddress;
        printf("loading ram\n");
        while (GetNextFileSector(&finfo, buffer, &count) == 0) {
            uint32_t *p = (uint32_t *)buffer;
            while (count > 0) {
                write_ram_long(addr, *p++);
                addr += sizeof(uint32_t);
                count -= sizeof(uint32_t);
            }
        }
    }
    
    // stop the sd driver
    if (sd_id >= 0)
        cogstop(sd_id);
        
    // replace this loader with vm_start.S
    printf("starting program\n");
    coginit(cogid(), _load_start_coguser3, (uint32_t)params);

    // should never reach this
    return 0;
}

#define CMD_MASK        0x03
#define WRITE_CMD       0x02
#define READ_CMD        0x03
#define ERASE_CHIP_CMD  0x01
#define ERASE_BLOCK_CMD 0x05
#define WRITE_DATA_CMD  0x09
#define SD_INIT_CMD     0x0d
#define SD_READ_CMD     0x11
#define SD_WRITE_CMD    0x15

static void write_ram_long(uint32_t addr, uint32_t val)
{
    xmm_mbox[0] = (addr & ~CMD_MASK) | WRITE_CMD;
    while (xmm_mbox[0])
        ;
    *(uint32_t *)(xmm_mbox[1] | (addr & cache_line_mask)) = val;
}

static uint32_t erase_flash_block(uint32_t addr)
{
    xmm_mbox[0] = ERASE_BLOCK_CMD | (addr << 8);
    while (xmm_mbox[0])
        ;
    return xmm_mbox[1];
}

static uint32_t write_flash_page(uint32_t addr, uint8_t *buffer, uint32_t count)
{
    uint32_t params[3];
    params[0] = (uint32_t)buffer;
    params[1] = count;
    params[2] = addr;
    xmm_mbox[0] =  WRITE_DATA_CMD | ((uint32_t)params << 8);
    while (xmm_mbox[0])
        ;
    return xmm_mbox[1];
}



