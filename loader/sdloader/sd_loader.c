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
#include "debug.h"

#define FILENAME        "AUTORUN PEX"

#define HUB_SIZE        (32 * 1024)

extern volatile uint32_t *sd_mbox;

#define XMEM_WRITE      0x8
#define XMEM_SIZE_16    0x1
#define XMEM_SIZE_32    0x2
#define XMEM_SIZE_64    0x3
#define XMEM_SIZE_128   0x4
#define XMEM_SIZE_256   0x5
#define XMEM_SIZE_512   0x6
#define XMEM_SIZE_1024  0x7
#define XMEM_END        0x8

/* cache driver communications mailbox */
typedef struct {
    volatile uint32_t hubaddr;
    volatile uint32_t extaddr;
} xmem_mbox_t;

/* this section contains the info structure */
SdLoaderInfo __attribute__((section(".coguser0"))) info_data;
extern unsigned int _load_start_coguser0[];

/* this section contains the external memory driver code */
uint32_t __attribute__((section(".coguser1"))) xmm_driver_data[496];
extern unsigned int _load_start_coguser1[];

/* this section contains the sd_driver code */
uint32_t __attribute__((section(".coguser2"))) sd_driver_data[496];
extern unsigned int _load_start_coguser2[];

/* kernel image buffer */
static uint8_t kernel_image[2048];

/* sector buffer for loading external memory */
static uint8_t padded_buffer[SECTOR_SIZE + 15];

/* external memory mailbox */
xmem_mbox_t *xmem_mbox;

static void write_block(uint32_t addr, uint8_t *buffer, int size);

int main(void)
{
    uint8_t *buffer = (uint8_t *)(((uint32_t)padded_buffer + 15) & ~15);
    SdLoaderInfo *info = (SdLoaderInfo *)_load_start_coguser0;
    uint32_t load_address, index_width, offset_width, addr, count;
    uint32_t tags_size, cache_size, cache_lines, cache_tags, mboxes, *sp;
    VolumeInfo vinfo;
    FileInfo finfo;
    PexeFileHdr *hdr;
    int sd_id, i;
    uint8_t *p;

    // determine the cache size and setup cache variables
    index_width = info->cache_geometry >> 8;
    offset_width = info->cache_geometry & 0xff;
    tags_size = (1 << index_width) * 4;
    cache_size = 1 << (index_width + offset_width);
    cache_lines = HUB_SIZE - cache_size;
    cache_tags = cache_lines - tags_size;
    mboxes = cache_tags - sizeof(xmem_mbox_t) * 8 - 4;
    xmem_mbox = (xmem_mbox_t *)mboxes;

    // load the external memory driver
    DPRINTF("Loading external memory driver\n");
    memset((void *)mboxes, 0, sizeof(xmem_mbox_t) * 8 + 4);
    xmem_mbox[8].hubaddr = XMEM_END;
    cognew(_load_start_coguser1, mboxes);
    
    DPRINTF("Loading SD driver\n");
    sd_mbox = (uint32_t *)mboxes - 2;
    sd_mbox[0] = 0xffffffff;
    if ((sd_id = cognew(_load_start_coguser2, sd_mbox)) < 0) {
        DPRINTF("Error loading SD driver\n");
        return 1;
    }
    while (sd_mbox[0])
        ;

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
	DPRINTF("Reading PEX file header\n");
	if (GetNextFileSector(&finfo, kernel_image, &count) != 0 || count < PEXE_HDR_SIZE) {
	    DPRINTF("Error reading PEX file header\n");
		return 1;
	}
		
	// verify the header
    DPRINTF("Verifying PEX file header\n");
    hdr = (PexeFileHdr *)kernel_image;
    if (strncmp(hdr->tag, PEXE_TAG, sizeof(hdr->tag)) != 0 || hdr->version != PEXE_VERSION) {
        DPRINTF("Bad PEX file header\n");
        return 1;
    }
	load_address = hdr->loadAddress;
	
	// move past the header
	memmove(kernel_image, (uint8_t *)kernel_image + PEXE_HDR_SIZE, SECTOR_SIZE - PEXE_HDR_SIZE);
	p = (uint8_t *)kernel_image + SECTOR_SIZE - PEXE_HDR_SIZE;
	
    // read the .kernel cog image
    DPRINTF("Reading kernel image\n");
    for (i = 1; i < 4; ++i) {
    	if (GetNextFileSector(&finfo, p, &count) != 0 || count < SECTOR_SIZE)
    		return 1;
        p += SECTOR_SIZE;
    }
    
    // load the image
    DPRINTF("Loading image at 0x%08x\n", load_address);
    addr = load_address;
    while (GetNextFileSector(&finfo, buffer, &count) == 0) {
        write_block(addr, buffer, XMEM_SIZE_512);
        addr += SECTOR_SIZE;
    }

    // stop the sd driver
    DPRINTF("Stopping SD driver\n");
    cogstop(sd_id);
    
    // setup the stack
    // at start stack contains xmem_mbox, cache_tags, cache_lines, cache_geometry, pc
    sp = (uint32_t *)mboxes;
    *--sp = load_address;
    *--sp = info->cache_geometry;
    *--sp = cache_lines;
    *--sp = cache_tags;
    *--sp = mboxes;

    // start the xmm kernel boot code
    DPRINTF("Starting kernel\n");
    coginit(cogid(), kernel_image, sp);

    // should never reach this
    return 0;
}

static void write_block(uint32_t addr, uint8_t *buffer, int size)
{
    xmem_mbox->extaddr = addr;
    xmem_mbox->hubaddr = (uint32_t)buffer | XMEM_WRITE | size;
    while (xmem_mbox->hubaddr)
        ;
}

