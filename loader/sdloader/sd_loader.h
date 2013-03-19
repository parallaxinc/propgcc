#ifndef __SDLOADER_H__
#define __SDLOADER_H__

#include <stdint.h>

/* flags */
#define SD_LOAD_RAM     (1 << 0)    // load ram for addresses above 0x30000000

/* sd loader information */
typedef struct {
    uint32_t cache_size;
    uint32_t cache_param1;
    uint32_t cache_param2;
    uint32_t cache_param3;
    uint32_t cache_param4;
    uint32_t flags;
} SdLoaderInfo;

#endif
