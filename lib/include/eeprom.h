#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <stdint.h>
#include <i2c.h>

typedef struct {
    I2C *dev;
    int address;
} EEPROM;

typedef struct {
    EEPROM state;
    I2C_COGDRIVER dev;
} EEPROM_COGDRIVER;

typedef struct {
    EEPROM state;
    I2C_SIMPLE dev;
} EEPROM_SIMPLE;

typedef struct {
    EEPROM state;
} EEPROM_BOOT;

/**
 * @brief Open an i2c EEPROM device
 *
 * @details Open an I2C EEPROM device using a COG driver.
 *
 * @param dev I2C EEPROM device structure to initialize
 * @param scl SCL pin number
 * @param sda SDA pin number
 * @param freq Bus frequency
 * @param address I2C EEPROM address in bits 7:1, zero in bit 0
 *
 * @returns a pointer to the EEPROM structure on success, NULL on failure.
 *
 */
EEPROM *eepromOpen(EEPROM_COGDRIVER *eeprom, int scl, int sda, int freq, int address);

/**
 * @brief Open an I2C EEPROM device
 *
 * @details Open an I2C EEPROM device using a simple driver that
 * runs on the COG of the caller.
 *
 * @param dev I2C EEPROM device structure to initialize
 * @param scl SCL pin number
 * @param sda SDA pin number
 * @param address I2C EEPROM address in bits 7:1, zero in bit 0
 *
 * @returns a pointer to the EEPROM structure on success, NULL on failure.
 *
 */
EEPROM *simple_eepromOpen(EEPROM_SIMPLE *eeprom, int scl, int sda, int address);

/**
 * @brief Open an EEPROM on the boot i2c bus on Propeller pins 28/29
 *
 * @details Use this function to open an EEPROM the i2c bus on pins 28/29 used by the
 * Propeller to boot from EEPROM.
 *
 * @param address I2C EEPROM address in bits 7:1, zero in bit 0
 *
 * @returns a pointer to an EEPROM structure or NULL on failure.
 */
EEPROM *eepromBootOpen(EEPROM_BOOT *eeprom, int address);

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

/**
 * @brief Close an EEPROM
 *
 * @details Close an EEPROM.
 *
 * @param eeprom Initialized EEPROM state structure
 *
 * @returns 0 on success, -1 on failure.
 *
 */
int eepromClose(EEPROM *eeprom);

#endif
