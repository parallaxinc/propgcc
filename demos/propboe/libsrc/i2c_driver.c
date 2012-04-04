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

#include <propeller.h>
#include "i2c_driver.h"

/* set high by allowing the pin to float high, set low by forcing it low */
#define i2c_float_scl_high() (DIRA &= ~scl_mask)
#define i2c_set_scl_low()    (DIRA |= scl_mask)
#define i2c_float_sda_high() (DIRA &= ~sda_mask)
#define i2c_set_sda_low()    (DIRA |= sda_mask)

/* i2c state information */
static _COGMEM uint32_t scl_mask;
static _COGMEM uint32_t sda_mask;
static _COGMEM volatile I2C_MAILBOX *mailbox;

static _NATIVE uint32_t i2cSend(void);
static _NATIVE uint32_t i2cReceive(void);

static _NATIVE void i2cStart(void);
static _NATIVE void i2cStop(void);
static _NATIVE int i2cSendByte( uint8_t byte);
static _NATIVE uint8_t i2cReceiveByte(int acknowledge);

_NAKED int main(void)
{
    I2C_INIT *init = (I2C_INIT *)PAR;
    I2C_CMD cmd;
    
    /* get the COG initialization parameters */
    scl_mask = init->scl_mask;
    sda_mask = init->sda_mask;
    mailbox = init->mailbox;
    
    /* tell the caller that we're done with initialization */
    mailbox->cmd = I2C_CMD_IDLE;
    
    /* initialize the i2c pins */
    DIRA &= scl_mask;
    DIRA &= sda_mask;
    OUTA &= scl_mask;
    OUTA &= sda_mask;
    
    /* handle requests */
    for (;;) {
        uint32_t sts;
    
        /* wait for the next request */
        while ((cmd = mailbox->cmd) == I2C_CMD_IDLE)
            ;
        
        /* dispatch on the command code */
        switch (cmd) {
        case I2C_CMD_SEND:
            sts = i2cSend();
            break;
        case I2C_CMD_RECEIVE:
            sts = i2cReceive();
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

static _NATIVE uint32_t i2cSend(void)
{
    uint8_t *p = mailbox->buffer;
    uint32_t count = mailbox->count;

    i2cStart();

    if (i2cSendByte(mailbox->hdr) != 0)
        return I2C_ERR_SEND_HDR;
        
    while (count > 0) {
        if (i2cSendByte(*p++) != 0)
            return I2C_ERR_SEND;
        --count;
    }

    i2cStop();

    return I2C_OK;
}

static _NATIVE uint32_t i2cReceive(void)
{
    uint8_t *p = mailbox->buffer;
    uint32_t count = mailbox->count;

    i2cStart();

    if (i2cSendByte(mailbox->hdr) != 0)
        return I2C_ERR_RECEIVE_HDR;
    
    while (count > 0) {
        int byte = i2cReceiveByte(count != 1);
        if (byte < 0)
            return I2C_ERR_RECEIVE;
        *p++ = byte;
        --count;
    }

    i2cStop();

    return I2C_OK;
}

static _NATIVE void i2cStart(void)
{
    i2c_set_sda_low();
    i2c_set_scl_low();
}

static _NATIVE void i2cStop(void)
{
    i2c_float_scl_high();
    i2c_float_sda_high();
}

static _NATIVE int i2cSendByte(uint8_t byte)
{
    int count, result;
    
    /* send the byte, high bit first */
    for (count = 8; --count >= 0; ) {
        if (byte & 0x80)
            i2c_float_sda_high();
        else
            i2c_set_sda_low();
        i2c_float_scl_high();
        i2c_set_scl_low();
        byte <<= 1;
    }
    
    /* receive the acknowledgement from the slave */
    i2c_float_sda_high();
    i2c_float_scl_high();
    result = (INA & sda_mask) != 0;
    i2c_set_scl_low();
    i2c_set_sda_low();
    
    return result;
}

static _NATIVE uint8_t i2cReceiveByte(int acknowledge)
{
    uint8_t byte = 0;
    int count;
    
    i2c_float_sda_high();
    
    for (count = 8; --count >= 0; ) {
        byte <<= 1;
        i2c_float_scl_high();
        byte |= (INA & sda_mask) ? 1 : 0;
        i2c_set_scl_low();
    }
    
    // acknowledge
    if (acknowledge)
        i2c_set_sda_low();
    else
        i2c_float_sda_high();
    i2c_float_scl_high();
    i2c_set_scl_low();
    i2c_set_sda_low();
    
    return byte;
}
