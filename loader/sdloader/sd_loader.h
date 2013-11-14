#ifndef __SDLOADER_H__
#define __SDLOADER_H__

#include <stdint.h>

/* flags */
#define SD_LOAD_RAM     (1 << 0)    // load ram for addresses above 0x30000000

/* sd loader information */
typedef struct {
    uint32_t cache_geometry;
    uint32_t flags;
} SdLoaderInfo;

#endif
