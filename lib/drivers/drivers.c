#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)

#include <stdint.h>
#include "propdev.h"

uint32_t __attribute__ ((section(".hub"))) hub_buffer[496];

int __attribute__((section(".hubtext"))) load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *param)
{
    if (codelen > 496) return -1;
    copy_from_xmm(hub_buffer, code, codelen);
    return cognew(hub_buffer, param);
}

#endif
