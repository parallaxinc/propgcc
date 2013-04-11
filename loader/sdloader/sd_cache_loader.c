/* sd_cache_loader.c - load a program to run the directly from the SD card

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

#define FILENAME    	"AUTORUN PEX"

#define HUB_SIZE    	(32 * 1024)

typedef struct {
	uint32_t	cacheptr_linemask;
	uint32_t	index_width;
	uint32_t	offset_width;
	uint32_t	cluster_width;
	uint32_t	cluster_map;
} CacheParams;

volatile uint32_t *xmm_mbox;
volatile uint32_t *vm_mbox;
uint32_t cache_line_mask;

/* this section contains the info structure */
SdLoaderInfo __attribute__((section(".coguser0"))) info_data;
extern unsigned int _load_start_coguser0[];

/* this section contains the cache driver code */
uint32_t __attribute__((section(".coguser1"))) xmm_driver_data[496];
extern unsigned int _load_start_coguser1[];

/* this section contains the vm_start.S code */
extern unsigned int _load_start_coguser3[];

static int ReadSector(char *tag, FileInfo *finfo, uint8_t *buf);

int main(void)
{
    uint8_t *buffer = (uint8_t *)_load_start_coguser1;
    //SdLoaderInfo *info = (SdLoaderInfo *)_load_start_coguser0;
    uint32_t cache_addr, vm_params[5], cluster_count, cluster, tmp;
    uint32_t load_address, *cluster_map;
    CacheParams cache_params;
    VolumeInfo vinfo;
    FileInfo finfo;
    PexeFileHdr *hdr;
    uint8_t *p;
    int i;
    
    cache_addr = HUB_SIZE - 8*1024; // info->cache_size;
    xmm_mbox = (volatile uint32_t *)(cache_addr - 2 * sizeof(uint32_t));
    vm_mbox = xmm_mbox - 1;
    
    // load the cache driver
    DPRINTF("Loading cache driver\n");
    xmm_mbox[0] = 0xffffffff;
    cognew(_load_start_coguser1, xmm_mbox);
    while (xmm_mbox[0])
        ;
        
    DPRINTF("Initializing SD card\n");
    if (SD_Init(xmm_mbox, 5) != 0) {
        DPRINTF("SD card initialization failed\n");
        return -1;
    }
        
    DPRINTF("Mounting SD filesystem\n");
    if (MountFS(buffer, &vinfo) != 0) {
        DPRINTF("MountFS failed\n");
        return 1;
    }
    
    // compute the cluster width
    cache_params.cluster_width = 0;
    if ((tmp = vinfo.sectorsPerCluster) != 0) {
    	for (; (tmp & 1) == 0; tmp >>= 1)
    		++cache_params.cluster_width;
    }
    	
    // open the .pex file
    DPRINTF("Opening AUTORUN.PEX\n");
    if (FindFile(buffer, &vinfo, FILENAME, &finfo) != 0) {
        DPRINTF("FindFile '%s' failed\n", FILENAME);
        return 1;
    }
    
    // compute the cluster count and allocate the cluster map
    cluster_count = (finfo.bytesRemaining + SECTOR_SIZE - 1) >> SECTOR_WIDTH;
    cluster_count = (cluster_count + vinfo.sectorsPerCluster - 1) >> cache_params.cluster_width;
	cluster_map = (uint32_t *)vm_mbox - cluster_count;
    
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
    DPRINTF("Loading kernel\n");
    vm_mbox[0] = 0;
    cognew(buffer, vm_mbox);
    vm_params[0] = load_address;
    
	// setup the cache parameters
    cache_params.cacheptr_linemask = cache_addr;
    cache_params.index_width = 4; 	// info->cache_param1;
    cache_params.offset_width = 9; 	// info->cache_param2;
	cache_params.cluster_map = (uint32_t)cluster_map;

    DPRINTF("Loading cluster map\n");
    cluster = finfo.cluster;
    for (i = 0; i < cluster_count; ++i) {
    	if (cluster >= vinfo.endOfClusterChain) {
    		DPRINTF("unexpected end of cluster chain\n");
			return 1;
    	}
    	cluster_map[i] = vinfo.firstDataSector + ((cluster - 2) << cache_params.cluster_width);
        if (GetFATEntry(&vinfo, buffer, cluster, &cluster) != 0) {
            DPRINTF("GetFATEntry %d failed\n", finfo.cluster);
            return 1;
        }
    }
	
    // initialize the cache
    DPRINTF("Initializing cache\n");
    xmm_mbox[0] = INIT_CACHE_CMD | ((uint32_t)&cache_params << 8);
    while (xmm_mbox[0])
        ;
    cache_line_mask = cache_params.cacheptr_linemask;
    
    // setup the parameters for vm_start.S
    vm_params[1] = (uint32_t)xmm_mbox;
    vm_params[2] = cache_line_mask;
    vm_params[3] = (uint32_t)vm_mbox;
    vm_params[4] = (uint32_t)cluster_map; // space below the cluster map is used as the stack
    
    // replace this loader with vm_start.S
    DPRINTF("Starting program\n");
    coginit(cogid(), _load_start_coguser3, vm_params);

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

