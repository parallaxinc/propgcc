/* xbeeload.h - definitions for the Xbee load driver */

#ifndef __XBEE_LOAD_H__
#define __XBEE_LOAD_H__

#include <stdint.h>
#include "xbeeframe.h"

typedef struct {
    XbeeFrame_t *mailbox;   // frame driver mailbox
    uint8_t *ldbuf;         // buffer containing initial data to load
    uint32_t ldcount;       // count of bytes in ldbuf
    uint8_t *ldaddr;        // hub load address
    uint32_t ldtotal;       // hub load byte count
    uint8_t *response;      // response
    uint32_t rcount;        // count of bytes in response
} XbeeLoadInit_t;

void XbeeLoad_start(XbeeLoadInit_t *init);

#endif
