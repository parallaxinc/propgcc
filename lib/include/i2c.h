/**
 * @file include/i2c.h
 * @brief Provides Propeller specific functions for I2C.
 *
 */

/*
 * Copyright (c) 2012 David Michael Betz
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 */

#ifndef __I2C_H__
#define __I2C_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include "i2c_driver.h"

/** forward type declarations */
typedef struct I2C I2C;

/* i2c operations */
typedef struct {
    int (*close)(I2C *dev);
    int (*read)(I2C *dev, int address, uint8_t *buffer, int count, int stop);
    int (*write)(I2C *dev, int address, uint8_t *buffer, int count, int stop);
} I2C_OPS;

/* i2c state information */
struct I2C {
    I2C_OPS *ops;
};

typedef struct {
    I2C i2c;
    int cog;
    volatile I2C_MAILBOX mailbox;
} I2C_COGDRIVER;

typedef struct {
    I2C i2c;
    uint32_t scl_mask;
    uint32_t sda_mask;
} I2C_SIMPLE;

/**
 * @brief Open an I2C device
 *
 * @details Open an I2C device using a COG driver.
 *
 * @param dev I2C device structure to initialize
 * @param scl SCL pin number
 * @param sda SDA pin number
 * @param freq Bus frequency
 *
 * @returns a pointer to the device on success, NULL on failure.
 *
 */
I2C *i2cOpen(I2C_COGDRIVER *dev, int scl, int sda, int freq);

/**
 * @brief Return a pointer to the I2C COG driver image
 *
 * @details Returns a pointer to the I2C COG driver image.
 * This allows the space that was occupied by the COG driver
 * to be reused after the driver has been loaded. Call this
 * only after the final call to i2cOpen.
 *
 * @returns a pointer to the I2C COG driver image.
 *
 */
void *i2cGetCogBuffer(void);

/**
 * @brief Open an I2C device
 *
 * @details Open an I2C device using a simple driver that
 * runs on the COG of the caller.
 *
 * @param dev I2C device structure to initialize
 * @param scl SCL pin number
 * @param sda SDA pin number
 *
 * @returns a pointer to the device on success, NULL on failure.
 *
 */
I2C *simple_i2cOpen(I2C_SIMPLE *dev, int scl, int sda);

/**
 * @brief Open the boot i2c bus on Propeller pins 28/29
 *
 * @details Use this function to open the i2c bus on pins 28/29 used by the
 * Propeller to boot from EEPROM.
 *
 * @returns a pointer to an I2C structure for the bus or NULL on failure.
 */
I2C *i2cBootOpen(void);

/**
 * @brief Get the address of the boot i2c bus COG driver
 *
 * @details Once the boot i2c bus has been opened with i2cBootOpen,
 * by loading a COG using either cognewFromBootEeprom or coginitFromBootEeprom,
 * or calling readBootEeprom or writeBootEeprom,
 * it is possible to reuse the hub memory space used by the boot i2c driver.
 * However, once this is done it will no longer be possible to load the boot
 * i2c driver again so after closing the boot i2c buss, calls to i2cBootOpen,
 * cognewFromBootEeprom, or coginitFromBootEeprom will fail.
 *
 * @returns a pointer to the boot i2c driver COG image for use as a buffer.
 */
void *i2cBootBuffer(void);

/**
 * @brief Close an I2C device
 *
 * @details Close an I2C device that was opened with
 * either i2cOpen or simple_i2cOpen.
 *
 * @param dev I2C device to close
 *
 * @returns 0 on success, -1 on failure.
 *
 */
static inline int i2cClose(I2C *dev)
{
    return (*dev->ops->close)(dev);
}

/**
 * @brief Write to an I2C device
 *
 * @details Write to an I2C device at the specified address.
 * The address should be the device address in bits 7:1 and
 * a zero in bit 0. If count is zero only the address byte
 * will be sent. Set the stop parameter to TRUE to cause an
 * I2C stop sequence to be emitted after the data. Setting it
 * to FALSE omits the stop sequence.
 *
 * @param dev I2C device to write to
 * @param address I2C address in bits 7:1, zero in bit 0
 * @param buffer Address of the buffer containing data to write
 * @param count Number of bytes of data to write
 * @param stop TRUE to send a stop sequence after the data
 *
 * @returns 0 on success, -1 on failure.
 *
 */
static inline int i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    return (*dev->ops->write)(dev, address, buffer, count, stop);
}

/**
 * @brief Read from an I2C device
 *
 * @details Read from an I2C device at the specified address.
 * The address should be the device address in bits 7:1 and
 * a zero in bit 0. Set the stop parameter to TRUE to cause an
 * I2C stop sequence to be emitted after the data. Setting it
 * to FALSE omits the stop sequence.
 *
 * @param dev I2C device to read from
 * @param address I2C address in bits 7:1, zero in bit 0
 * @param buffer Address of the buffer to receive data
 * @param count Number of bytes of data to receive
 * @param stop TRUE to send a stop sequence after the data
 *
 * @returns 0 on success, -1 on failure.
 *
 */
static inline int i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop)
{
    return (*dev->ops->read)(dev, address, buffer, count, stop);
}

/* internal functions */
int cog_i2cRead(I2C *dev, int address, uint8_t *buffer, int count, int stop);
int cog_i2cWrite(I2C *dev, int address, uint8_t *buffer, int count, int stop);

#if defined(__cplusplus)
}
#endif

#endif
