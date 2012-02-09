#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)

#include <stdint.h>
#include <string.h>
#include "propdev.h"

uint32_t __attribute__ ((section(".hub"))) _hub_buffer[496];

int __attribute__((section(".hubtext"))) load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *param)
{
    if (codelen > 496) return -1;
    memcpy(_hub_buffer, code, codelen*4);
    return cognew(_hub_buffer, param);
}

#endif
