#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>
#include <i2c.h>

typedef struct {
    I2C *dev;
    int address;
} EEPROM;

/**
 * @brief Initialize an EEPROM state structure
 *
 * @details Initialize an EEPROM state structure with an
 * open i2c device and an i2c address.
 *
 * @param eeprom EEPROM state structure to initialize
 * @param address I2C address in bits 7:1, zero in bit 0
 *
 */

void eepromInit(EEPROM *eeprom, I2C *dev, int address);

/**
 * @brief Read from an EEPROM
 *
 * @details Read data from an EEPROM.
 *
 * @param eeprom Initialized EEPROM state structure
 * @param address EEPROM address to read
 * @param buffer Address of the buffer to receive data
 * @param count Number of bytes of data to read
 *
 * @returns 0 on success, -1 on failure.
 *
 */
int eepromRead(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count);

/**
 * @brief Write to an EEPROM
 *
 * @details Write data to an EEPROM.
 *
 * @param eeprom Initialized EEPROM state structure
 * @param address EEPROM address to write
 * @param buffer Address of the buffer containing data to write
 * @param count Number of bytes of data to write
 *
 * @returns 0 on success, -1 on failure.
 *
 */
int eepromWrite(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count);

#endif
