#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <propeller.h>
#include "fatread.h"
#include "sdio.h"
#include "../src/pex.h"
#include "sd_loader.h"

#define FILENAME    	"AUTORUN PEX"

#define HUB_SIZE    	(32 * 1024)

#define CMD_MASK        0x03
#define WRITE_CMD       0x02
#define READ_CMD        0x03
#define INIT_CACHE_CMD	0x19

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

/* this section contains the xmm_driver code */
uint32_t __attribute__((section(".coguser1"))) xmm_driver_data[496];
extern unsigned int _load_start_coguser1[];

/* this section contains the vm_start.S code */
extern unsigned int _load_start_coguser3[];

int main(void)
{
    uint8_t *buffer = (uint8_t *)xmm_driver_data;
//    SdLoaderInfo *info = (SdLoaderInfo *)_load_start_coguser0;
    uint32_t cache_addr, vm_params[5], cluster_count, count, cluster, tmp, *cluster_map;
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
    printf("loading cache driver\n");
    xmm_mbox[0] = 0xffffffff;
    cognew(_load_start_coguser1, xmm_mbox);
    while (xmm_mbox[0])
        ;
        
    if (SD_Init(xmm_mbox, 5) != 0) {
        printf("SD card initialization failed\n");
        return -1;
    }
        
    if (MountFS(buffer, 5, &vinfo) != 0) {
        printf("MountFS failed\n");
        return 1;
    }
    
    // compute the cluster width
    cache_params.cluster_width = 0;
    for (tmp = vinfo.sectorsPerCluster; (tmp & 1) == 0; tmp >>= 1)
    	++cache_params.cluster_width;
    	
    // open the .pex file
    if (FindFile(buffer, &vinfo, FILENAME, &finfo) != 0) {
        printf("FindFile '%s' failed\n", FILENAME);
        return 1;
    }
    
    // compute the cluster count and allocate the cluster map
    cluster_count = (finfo.bytesRemaining + SECTOR_SIZE - 1) >> SECTOR_WIDTH;
    cluster_count = (cluster_count + vinfo.sectorsPerCluster - 1) >> cache_params.cluster_width;
	cluster_map = (uint32_t *)vm_mbox - cluster_count;
    
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
    printf("loading kernel\n");
    hdr = (PexeFileHdr *)buffer;
    vm_mbox[0] = 0;
    cognew(hdr->kernel, vm_mbox);
    vm_params[0] = hdr->loadAddress;
    
	// setup the cache parameters
    cache_params.cacheptr_linemask = cache_addr;
    cache_params.index_width = 4; 	// info->cache_param1;
    cache_params.offset_width = 9; 	// info->cache_param2;
	cache_params.cluster_map = (uint32_t)cluster_map;

    printf("loading cluster map\n");
    cluster = finfo.cluster;
    for (i = 0; i < cluster_count; ++i) {
    	if (cluster >= vinfo.endOfClusterChain) {
#ifdef DEBUG
    		printf("unexpected end of cluster chain\n");
#endif
			return 1;
    	}
    	cluster_map[i] = vinfo.firstDataSector + ((cluster - 2) << cache_params.cluster_width);
        if (GetFATEntry(&vinfo, buffer, cluster, &cluster) != 0) {
#ifdef DEBUG
            printf("GetFATEntry %d failed\n", finfo->cluster);
#endif
            return 1;
        }
    }
	
    // initialize the cache
    printf("initializing cache\n");
    xmm_mbox[0] = INIT_CACHE_CMD | ((uint32_t)&cache_params << 8);
    while (xmm_mbox[0])
        ;
    cache_line_mask = cache_params.cacheptr_linemask;
    
    // setup the parameters for vm_start.S
    vm_params[1] = (uint32_t)xmm_mbox;
    vm_params[2] = cache_line_mask;
    vm_params[3] = (uint32_t)vm_mbox;
    vm_params[4] = (uint32_t)cluster_map;
    
    // replace this loader with vm_start.S
    printf("starting program\n");
    coginit(cogid(), _load_start_coguser3, vm_params);

    // should never reach this
    return 0;
}
