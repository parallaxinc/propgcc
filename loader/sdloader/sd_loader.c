/* sd_loader.c - load a program from an SD card into external memory

Copyright (c) 2011 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "fatread.h"
#include "sdio.h"
#include "../src/pex.h"
#include "sd_loader.h"
#include "cacheops.h"
#include "debug.h"

#define FILENAME    "AUTORUN PEX"

#define HUB_SIZE    (32 * 1024)

volatile uint32_t *xmm_mbox;
volatile uint32_t *sd_mbox;
volatile uint32_t *vm_mbox;
uint32_t cache_line_mask;

/* this section contains the info structure */
SdLoaderInfo __attribute__((section(".coguser0"))) info_data;
extern unsigned int _load_start_coguser0[];

/* this section contains the cache driver code */
uint32_t __attribute__((section(".coguser1"))) xmm_driver_data[496];
extern unsigned int _load_start_coguser1[];

/* this section contains the sd_driver code */
uint32_t __attribute__((section(".coguser2"))) sd_driver_data[496];
extern unsigned int _load_start_coguser2[];

/* this section contains the vm_start.S code */
extern unsigned int _load_start_coguser3[];

static int ReadSector(char *tag, FileInfo *finfo, uint8_t *buf);
static void write_ram_long(uint32_t addr, uint32_t val);
static uint32_t erase_flash_block(uint32_t addr);
static uint32_t write_flash_page(uint32_t addr, uint8_t *buffer, uint32_t count);

int main(void)
{
    uint8_t *buffer = (uint8_t *)_load_start_coguser1;
    SdLoaderInfo *info = (SdLoaderInfo *)_load_start_coguser0;
    uint32_t cache_addr, load_address;
    uint32_t params[5];
    VolumeInfo vinfo;
    FileInfo finfo;
    PexeFileHdr *hdr;
    uint32_t count;
    int sd_id, i;
    uint8_t *p;
    
    cache_addr = HUB_SIZE - info->cache_size;
    xmm_mbox = (uint32_t *)(cache_addr - 2 * sizeof(uint32_t));
    vm_mbox = xmm_mbox - 1;
    
    DPRINTF("Loading SD driver\n");
    sd_mbox = vm_mbox - 2;
    sd_mbox[0] = 0xffffffff;
    sd_id = cognew(_load_start_coguser2, sd_mbox);
    while (sd_mbox[0])
        ;
    
    params[0] = (uint32_t)xmm_mbox;
    params[1] = cache_addr;
    params[2] = info->cache_param1;
    params[3] = info->cache_param2;
    
    // load the cache driver
    DPRINTF("Loading cache driver\n");
    xmm_mbox[0] = 0xffffffff;
    cognew(_load_start_coguser1, params);
    while (xmm_mbox[0])
        ;
    cache_line_mask = params[0];
        
    DPRINTF("Initializing SD card\n");
    if (SD_Init(sd_mbox, 5) != 0) {
        DPRINTF("SD card initialization failed\n");
        return 1;
    }
        
    DPRINTF("Mounting filesystem\n");
    if (MountFS(buffer, &vinfo) != 0) {
        DPRINTF("MountFS failed\n");
        return 1;
    }
    
    // open the .pex file
    DPRINTF("Opening 'autorun.pex'\n");
    if (FindFile(buffer, &vinfo, FILENAME, &finfo) != 0) {
        DPRINTF("FindFile '%s' failed\n", FILENAME);
        return 1;
    }
    
	// read the file header
	if (ReadSector("PEX file header", &finfo, buffer) != 0)
		return 1;
		
	// verify the header
    hdr = (PexeFileHdr *)buffer;
    if (strncmp(hdr->tag, PEXE_TAG, sizeof(hdr->tag)) != 0 || hdr->version != PEXE_VERSION) {
        DPRINTF("Bad PEX file header\n");
        return 1;
    }
	load_address = hdr->loadAddress;
	
	// move past the header
	memmove(buffer, buffer + PEXE_HDR_SIZE, SECTOR_SIZE - PEXE_HDR_SIZE);
	p = buffer + SECTOR_SIZE - PEXE_HDR_SIZE;
	
    // read the .kernel cog image
    for (i = 1; i < 4; ++i) {
    	if (ReadSector("kernel image", &finfo, p) != 0)
    		return 1;
        p += SECTOR_SIZE;
    }
    
    // start the xmm kernel
    DPRINTF("Starting kernel\n");
    hdr = (PexeFileHdr *)buffer;
    vm_mbox[0] = 0;
    cognew(buffer, vm_mbox);
    
    // setup the parameters for vm_start.S
    params[0] = load_address;
    params[1] = (uint32_t)xmm_mbox;
    params[2] = cache_line_mask;
    params[3] = (uint32_t)vm_mbox;
    params[4] = (uint32_t)vm_mbox;

    // load into ram
    if (load_address < 0x30000000 || (info->flags & SD_LOAD_RAM)) {
        uint32_t addr = load_address;
        DPRINTF("Loading RAM at 0x%08x\n", load_address);
        while (GetNextFileSector(&finfo, buffer, &count) == 0) {
            uint32_t *p = (uint32_t *)buffer;
            while (count > 0) {
                write_ram_long(addr, *p++);
                addr += sizeof(uint32_t);
                count -= sizeof(uint32_t);
            }
        }
    }

    // load into flash/eeprom
    else {
        uint32_t addr = 0x00000000;
        DPRINTF("Loading flash/EEPROM at 0x%08x\n", load_address);
        while (GetNextFileSector(&finfo, buffer, &count) == 0) {
            if ((addr & 0x00000fff) == 0)
                erase_flash_block(addr);
            write_flash_page(addr, buffer, count);
            addr += SECTOR_SIZE;
        }
    }
        
    // stop the sd driver
    if (sd_id >= 0) {
        DPRINTF("Stopping the SD driver\n");
        cogstop(sd_id);
    }
        
    // replace this loader with vm_start.S
    DPRINTF("Starting program\n");
    coginit(cogid(), _load_start_coguser3, (uint32_t)params);

    // should never reach this
    return 0;
}

static int ReadSector(char *tag, FileInfo *finfo, uint8_t *buf)
{
	uint32_t count;
	if (GetNextFileSector(finfo, buf, &count) != 0 || count != SECTOR_SIZE) {
		DPRINTF("Incomplete %s\n", tag);
		return 1;
	}
	return 0;
}

static void write_ram_long(uint32_t addr, uint32_t val)
{
    xmm_mbox[0] = (addr & ~CMD_MASK) | WRITE_CMD;
    while (xmm_mbox[0])
        ;
    *(uint32_t *)(xmm_mbox[1] + (addr & cache_line_mask)) = val;
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



