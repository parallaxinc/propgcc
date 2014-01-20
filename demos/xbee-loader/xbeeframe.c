/* xbeeframe.c - Xbee frame driver */

#include <propeller.h>
#include "xbeeframe.h"

/**
 * XbeeFrame_start - initializes and starts native assembly driver in a cog.
 * @param init is the initialization structure
 * @param mailbox is the mailbox structure
 * @param rxpin is pin number for receive input
 * @param txpin is pin number for transmit output
 * @param mode is interface mode. see header XBEEFRAME_MODE_...
 * @param baudrate is frequency of bits ... 115200, 57600, etc...
 * @returns COG ID on success and -1 on failure
 */
int XbeeFrame_start(XbeeFrameInit_t *init, XbeeFrame_t *mailbox, int rxpin, int txpin, int mode, int baudrate)
{
    use_cog_driver(xbeeframe_driver);

    init->mailbox  = mailbox;               // mailbox
    init->rx_pin   = rxpin;                 // receive pin
    init->tx_pin   = txpin;                 // transmit pin
    init->mode     = mode;                  // interface mode
    init->ticks    = _clkfreq / baudrate;   // baud
    init->rxlength = XBEEFRAME_RXSIZE;      // receive buffer length
    init->buffers  = mailbox->buffers;      // receive buffers
    
    memset(mailbox, 0, sizeof(XbeeFrame_t));
    mailbox->cogId = load_cog_driver(xbeeframe_driver, init);

    return mailbox->cogId;
}

/**
 * XbeeFrame_sendframe - sends a frame to the Xbee module 
 */
void XbeeFrame_sendframe(XbeeFrame_t *mailbox, uint8_t *frame, int length)
{
    while (mailbox->txstatus != XBEEFRAME_STATUS_IDLE)
        ;
    mailbox->txframe = frame;
    mailbox->txlength = length;
    mailbox->txstatus = XBEEFRAME_STATUS_BUSY;
}

/**
 * XbeeFrame_recvframe - receives a frame from the Xbee module 
 */
uint8_t *XbeeFrame_recvframe(XbeeFrame_t *mailbox, int *plength)
{
    if (mailbox->rxstatus == XBEEFRAME_STATUS_IDLE)
        return NULL;
    *plength = (int)mailbox->rxlength;
    return (uint8_t *)mailbox->rxframe;
}

/**
 * XbeeFrame_release - releases a frame received from the Xbee module 
 */
void XbeeFrame_release(XbeeFrame_t *mailbox)
{
    mailbox->rxstatus = XBEEFRAME_STATUS_IDLE;
}
