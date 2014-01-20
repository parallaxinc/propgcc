/* xbeeframe.h - definitions for the Xbee frame driver */

#ifndef __XBEEFRAME_H__
#define __XBEEFRAME_H__

#include <stdint.h>

/**
 * Defines receive buffer length
 */
#define XBEEFRAME_RXSIZE    1024

/**
 * Defines mode bits
 *   mode bit 0 = invert rx
 *   mode bit 1 = invert tx
 *   mode bit 2 = open-drain/source tx
 */
#define XBEEFRAME_MODE_INVERT_RX        1
#define XBEEFRAME_MODE_INVERT_TX        2
#define XBEEFRAME_MODE_OPENDRAIN_TX     4

#define XBEEFRAME_STATUS_IDLE           0
#define XBEEFRAME_STATUS_BUSY           1

/* mailbox structure */
typedef struct {
    volatile uint8_t *rxframe;  // rx frame buffer
    volatile uint32_t rxlength; // rx frame length
    volatile uint32_t rxstatus; // rx status
    volatile uint8_t *txframe;  // tx frame buffer
    volatile uint32_t txlength; // tx frame length
    volatile uint32_t txstatus; // tx status
    volatile uint32_t ticks;    // number of ticks per bit
    uint32_t cogId;             // cog running driver (not used by driver)
    uint8_t buffers[XBEEFRAME_RXSIZE * 2];
} XbeeFrame_t;

/* init structure */
typedef struct {
    XbeeFrame_t *mailbox;       // mailbox address
    uint32_t rx_pin;            // receive pin
    uint32_t tx_pin;            // transmit pin
    uint32_t mode;              // rx/tx mode
    uint32_t ticks;             // baud rate
    uint32_t rxlength;          // size of frame buffer
    uint8_t *buffers;           // rxlength*2 size buffer
} XbeeFrameInit_t;

int XbeeFrame_start(XbeeFrameInit_t *init, XbeeFrame_t *mailbox, int rxpin, int txpin, int mode, int baudrate);
void XbeeFrame_sendframe(XbeeFrame_t *mailbox, uint8_t *frame, int length);
uint8_t *XbeeFrame_recvframe(XbeeFrame_t *mailbox, int *plength);
void XbeeFrame_release(XbeeFrame_t *mailbox);

#endif
