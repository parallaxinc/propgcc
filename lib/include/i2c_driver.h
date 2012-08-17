/**
 * @file include/i2c_driver.h - i2c single master driver definitions
 * @brief Declarations here are not meant as public API.
 */

/*
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

#ifndef __I2C_DRIVER_H__
#define __I2C_DRIVER_H__

#include <stdint.h>

typedef enum {
    I2C_CMD_IDLE,
    I2C_CMD_INIT,
    I2C_CMD_SEND,
    I2C_CMD_RECEIVE
} I2C_CMD;

typedef enum {
    I2C_OK = 0,
    I2C_ERR_UNKNOWN_CMD,
    I2C_ERR_SEND_HDR,
    I2C_ERR_SEND,
    I2C_ERR_RECEIVE_HDR,
    I2C_ERR_RECEIVE
} I2C_RESULT;

typedef struct {
    uint32_t cmd;
    uint32_t sts;
    uint32_t hdr;
    uint8_t *buffer;
    uint32_t count;
    uint32_t stop;
} I2C_MAILBOX;

typedef struct {
    uint32_t scl;
    uint32_t sda;
    uint32_t ticks_per_cycle;
    volatile I2C_MAILBOX *mailbox;
} I2C_INIT;

#define I2C_READ        1
#define I2C_WRITE       0

#endif
