/* xbeeload.c - Xbee load driver interface */

#include <propeller.h>
#include "xbeeload.h"

/**
 * XbeeLoad_start - initializes and starts xbee load driver in the current cog.
 * @param init is the initialization structure
 * @returns never
 */
void XbeeLoad_start(XbeeLoadInit_t *init)
{
    extern uint32_t binary_xbeeload_driver_dat_start[];
    coginit(cogid(), binary_xbeeload_driver_dat_start, init);
}
