/* kernel.c - a dummy debug kernel for the gdb stub used for initial downloads

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

#include <stdint.h>
#include "propeller.h"

/* attention byte sent before a command is sent */
#define ATTN        0x01
#define ACK         0x40

/* serial debug kernel function codes */
#define FCN_STEP    0x01
#define FCN_RUN     0x02
#define FCN_READ    0x03
#define FCN_WRITE   0x04

/* code received from the serial debug kernel when the program halts due to a breakpoint or single step */
#define HALT        '!'

static unsigned int txmask = 1 << 30;
static unsigned int rxmask = 1 << 31;
static unsigned int bitcycles = 80000000 / 115200;

static _COGMEM uint32_t addr;
static _COGMEM uint32_t len;
static _COGMEM uint32_t chksum;
static _COGMEM uint32_t byte;

static _NATIVE void rx_packet(void);
static _NATIVE void tx_packet(void);
static _NATIVE void txbyte(int c);
static _NATIVE int rxbyte(void);

_NAKED int main(void)
{
    /* setup rx/tx pins */
    _OUTA |= txmask;
    _DIRA |= txmask;
    _DIRA &= ~rxmask;
    
    for (;;) {
        
        txbyte(HALT);
        
        while (rxbyte() != ATTN)
            ;
        txbyte(ACK);
        
        switch (rxbyte()) {
            case FCN_STEP:
            case FCN_RUN:
	        /* this starts the Spin interpreter in ROM (at address 0xf004) */
                coginit(cogid(), 0xf004, 0x4);
                break;
            case FCN_READ:
                tx_packet();
                break;
            case FCN_WRITE:
                rx_packet();
                break;
        }
    }
    
    return 0;
}

static _NATIVE void get_addr_len(void)
{
    addr = rxbyte();
    byte = rxbyte();
    addr |= byte << 8;
    len = rxbyte();
    chksum = len;
}

static _NATIVE void rx_packet(void)
{
    get_addr_len();
    for (; len > 0; --len) {
        byte = rxbyte();
        *(uint8_t *)addr = byte;
        chksum += byte;
        ++addr;
    }
    txbyte(chksum);
}

static _NATIVE void hex(uint32_t value)
{
    int shift = 28;
    do {
        uint8_t nibble = (value >> shift) & 0xf;
        txbyte((nibble < 10 ? '0' : 'a' - 10) + nibble);
        shift -= 4;
    } while (shift >= 0);
    
    txbyte('\r');
    txbyte('\n');
}

static _NATIVE void tx_packet(void)
{
    get_addr_len();
    for (; len > 0; --len) {
        byte = *(uint8_t *)addr;
        chksum += byte;
        txbyte(byte);
        ++addr;
    }
    txbyte(chksum);
}

static _NATIVE void txbyte(int c)
{
    unsigned int waitcycles;
    int i, value;
    
    value = (c | 256) << 1;
    waitcycles = _CNT + bitcycles;
    for (i = 0; i < 10; i++)
    {
        waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
        if (value & 1)
            _OUTA |= txmask;
        else
            _OUTA &= ~txmask;
        value >>= 1;
    }
}

int _NATIVE rxbyte(void)
{
    unsigned int waitcycles;
    int i, value;
    
    /* wait for a start bit */
    __builtin_propeller_waitpeq(0, rxmask);
    
    /* sync for one half bit */
    waitcycles = _CNT + (bitcycles>>1) + bitcycles;
    value = 0;
    for (i = 0; i < 8; i++) {
        waitcycles = __builtin_propeller_waitcnt(waitcycles, bitcycles);
        value = ( (0 != (_INA & rxmask)) << 7) | (value >> 1);
    }
    __builtin_propeller_waitpeq(rxmask, rxmask);
    return value;
}
