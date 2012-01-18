#ifndef __SDLOADER_H__
#define __SDLOADER_H__

#include <stdint.h>

/* sd loader information */
typedef struct {
    uint32_t cache_size;
    uint32_t cache_param1;
    uint32_t cache_param2;
    uint32_t use_cache_driver_for_sd;
} SdLoaderInfo;

#endif
