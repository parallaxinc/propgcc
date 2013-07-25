/* i2c_driver.c - i2c single master driver

Copyright (c) 2012 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef __PROPELLER2__

#include <propeller.h>
#include "i2c_driver.h"

/* minimum overhead per half cycle */
#define MINIMUM_OVERHEAD    32

#ifdef PARALLAX_I2C_BUS

/* set sda high by allowing it to float high, set low by forcing it low */
/* actively drive scl */
#define i2c_set_scl_high()  (OUTA |= scl_mask)
#define i2c_set_scl_low()   (OUTA &= ~scl_mask)
#define i2c_set_sda_high()  (DIRA &= ~sda_mask)
#define i2c_set_sda_low()   (DIRA |= sda_mask)

#else

/* set high by allowing the pin to float high, set low by forcing it low */
#define i2c_set_scl_high() (DIRA &= ~scl_mask)
#define i2c_set_scl_low()    (DIRA |= scl_mask)
#define i2c_set_sda_high() (DIRA &= ~sda_mask)
#define i2c_set_sda_low()    (DIRA |= sda_mask)

#endif

/* i2c state information */
static _COGMEM uint32_t scl_mask;
static _COGMEM uint32_t sda_mask;
static _COGMEM uint32_t half_cycle;
static _COGMEM volatile I2C_MAILBOX *mailbox;

static _NATIVE void i2cStart(void);
static _NATIVE void i2cStop(void);
static _NATIVE int i2cSendByte( uint8_t byte);
static _NATIVE uint8_t i2cReceiveByte(int acknowledge);

_NAKED int main(void)
{
    I2C_INIT *init = (I2C_INIT *)PAR;
    I2C_CMD cmd;
    uint8_t *p;
    uint32_t count;
    
    /* get the COG initialization parameters */
    scl_mask = 1 << init->scl;
    sda_mask = 1 << init->sda;
    half_cycle = init->ticks_per_cycle >> 1;
    mailbox = init->mailbox;
    
    /* make sure the delta doesn't get too small */
    if (half_cycle > MINIMUM_OVERHEAD)
        half_cycle -= MINIMUM_OVERHEAD;
    
    /* tell the caller that we're done with initialization */
    mailbox->cmd = I2C_CMD_IDLE;
    
#ifdef PARALLAX_I2C_BUS

    /* initialize the i2c pins */
    OUTA |= scl_mask;
    OUTA &= ~sda_mask;
    DIRA |= scl_mask;
    DIRA &= ~sda_mask;
    
#else
    
    /* initialize the i2c pins */
    DIRA &= ~scl_mask;
    DIRA &= ~sda_mask;
    OUTA &= ~scl_mask;
    OUTA &= ~sda_mask;

#endif

    /* handle requests */
    for (;;) {
        uint32_t sts;
    
        /* wait for the next request */
        while ((cmd = mailbox->cmd) == I2C_CMD_IDLE)
            ;
        
        /* dispatch on the command code */
        switch (cmd) {
        case I2C_CMD_SEND:
        case I2C_CMD_SEND_MORE:
            p = mailbox->buffer;
            count = mailbox->count;
            sts = I2C_OK;
            if (cmd == I2C_CMD_SEND) {
                i2cStart();
                if (i2cSendByte(mailbox->hdr) != 0) {
                    sts = I2C_ERR_SEND_HDR;
                    break;
                }
            }
            while (count > 0) {
                if (i2cSendByte(*p++) != 0) {
                    sts = I2C_ERR_SEND;
                    break;
                }
                --count;
            }
            if (mailbox->stop)
                i2cStop();
            break;
        case I2C_CMD_RECEIVE:
        case I2C_CMD_RECEIVE_MORE:
            p = mailbox->buffer;
            count = mailbox->count;
            sts = I2C_OK;
            if (cmd == I2C_CMD_RECEIVE) {
                i2cStart();
                if (i2cSendByte(mailbox->hdr) != 0) {
                    sts = I2C_ERR_RECEIVE_HDR;
                    break;
                }
            }
            while (count > 0) {
                int byte = i2cReceiveByte(count != 1);
                if (byte < 0) {
                    sts = I2C_ERR_RECEIVE;
                    break;
                }
                *p++ = byte;
                --count;
            }
            if (mailbox->stop)
                i2cStop();
            break;
        default:
            sts = I2C_ERR_UNKNOWN_CMD;
            break;
        }
        
        mailbox->sts = sts;
        mailbox->cmd = I2C_CMD_IDLE;
    }
    
    return 0;
}

static _NATIVE void i2cStart(void)
{
    i2c_set_scl_high();
    i2c_set_sda_high();
    waitcnt(CNT + half_cycle);
    i2c_set_sda_low();
    waitcnt(CNT + half_cycle);
    i2c_set_scl_low();
}

static _NATIVE void i2cStop(void)
{
    /* scl and sda should be low on entry */
    waitcnt(CNT + half_cycle);
    i2c_set_scl_high();
    i2c_set_sda_high();
}

static _NATIVE int i2cSendByte(uint8_t byte)
{
    int count, result;
    
    /* send the byte, high bit first */
    for (count = 8; --count >= 0; ) {
        if (byte & 0x80)
            i2c_set_sda_high();
        else
            i2c_set_sda_low();
        waitcnt(CNT + half_cycle);
        i2c_set_scl_high();
        waitcnt(CNT + half_cycle);
        i2c_set_scl_low();
        byte <<= 1;
    }
    
    /* receive the acknowledgement from the slave */
    i2c_set_sda_high();
    waitcnt(CNT + half_cycle);
    i2c_set_scl_high();
    result = (INA & sda_mask) != 0;
    waitcnt(CNT + half_cycle);
    i2c_set_scl_low();
    i2c_set_sda_low();
    
    return result;
}

static _NATIVE uint8_t i2cReceiveByte(int acknowledge)
{
    uint8_t byte = 0;
    int count;
    
    i2c_set_sda_high();
    
    for (count = 8; --count >= 0; ) {
        byte <<= 1;
        waitcnt(CNT + half_cycle);
        i2c_set_scl_high();
        byte |= (INA & sda_mask) ? 1 : 0;
        waitcnt(CNT + half_cycle);
        i2c_set_scl_low();
    }
    
    // acknowledge
    if (acknowledge)
        i2c_set_sda_low();
    else
        i2c_set_sda_high();
    waitcnt(CNT + half_cycle);
    i2c_set_scl_high();
    waitcnt(CNT + half_cycle);
    i2c_set_scl_low();
    i2c_set_sda_low();
    
    return byte;
}

#endif // __PROPELLER2__
