#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)

#include <stdint.h>
#include <string.h>
#include <propeller.h>

#define BUS_LOCK_CMD 0x1D

uint32_t __attribute__ ((section(".hub"))) _hub_buffer[496];
extern uint16_t _xmm_mbox_p;

uint32_t *get_cog_driver_xmm(uint32_t *code, uint32_t codelen)
{
    if (codelen > 496) codelen = 496;
    memcpy(_hub_buffer, code, codelen * sizeof(uint32_t));
    return _hub_buffer;
}

int load_cog_driver_xmm(uint32_t *code, uint32_t codelen, uint32_t *param)
{
    if (codelen > 496) codelen = 496;
    memcpy(_hub_buffer, code, codelen * sizeof(uint32_t));
    return cognew(_hub_buffer, param);
}

// This routine sends a command to the kernel's cache driver and returns the result
// It must be loaded in HUB RAM so we don't generate a cache miss
static uint32_t __attribute__((section(".hubtext"))) do_cache_cmd(uint32_t cmd)
{
    volatile uint32_t *xmm_mbox = (uint32_t *)(uint32_t)_xmm_mbox_p;
    xmm_mbox[0] = cmd;
    while (xmm_mbox[0])
        ;
    return xmm_mbox[1];
}

// This routine passes a bus lock to the cache driver
void kernel_use_lock(uint32_t lockId)
{
    do_cache_cmd(BUS_LOCK_CMD | (lockId << 8));
}

#endif
