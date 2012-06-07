/**
 * @file include/cogload.h
 * @brief Provides Propeller specific functions to load COGs from EEPROM.
 *
 * Copyright (c) 2011-2012 by Parallax, Inc.
 * MIT Licensed
 */

#ifndef __COGLOAD_H__
#define __COGLOAD_H__

#include <stdint.h>
#include <i2c.h>

/**
 * @brief Start a new Propeller COG from a COG image in EEPROM
 *
 * @details This is used to start a COG with code compiled as PASM, AS, or COG-C
 * when the COG image is in EEPROM.
 * PASM can be any Spin/PASM code that is self-contained. That is, all data
 * for initialization and mailbox use are passed via the par parameter.
 * Changing PASM variables from SPIN code will not work with this method.
 *
 * @details GAS and COG-C programs have similar restrictions.
 * COG-C programs should not use any stack or variables in HUB memory that
 * are not accessed via PAR mailbox or pointers.
 *
 * @details This function
 * requires the boot i2c COG driver so it must either already be loaded
 * or the memory space used by it cannot have been overwritten by calling
 * the i2cBootBuffer function.
 *
 * @param code Address of PASM to load
 * @param size of code in bytes
 * @param param Value of par parameter usually an address
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
int cognewFromBootEeprom(void *code, size_t codeSize, void *param);

/**
 * @brief Start a specified Propeller COG from a COG image in EEPROM
 *
 * @details This is used to start a COG with code compiled as PASM, AS, or COG-C
 * when the COG image is in EEPROM.
 * PASM can be any Spin/PASM code that is self-contained. That is, all data
 * for initialization and mailbox use are passed via the par parameter.
 * Changing PASM variables from SPIN code will not work with this method.
 *
 * @details GAS and COG-C programs have similar restrictions.
 * COG-C programs should not use any stack or variables in HUB memory that
 * are not accessed via PAR mailbox or pointers.
 *
 * @details This function
 * requires the boot i2c COG driver so it must either already be loaded
 * or the memory space used by it cannot have been overwritten by calling
 * the i2cBootBuffer function.
 *
 * @param id COG id of to load
 * @param code Address of PASM to load
 * @param size of code in bytes
 * @param param Value of par parameter usually an address
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
int coginitFromBootEeprom(int id, void *code, size_t codeSize, void *param);

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
 * @brief Read from the boot EEPROM
 *
 * @details Use this function to read from the boot EEPROM. 
 *
 * @details This function
 * requires the boot i2c COG driver so it must either already be loaded
 * or the memory space used by it cannot have been overwritten by calling
 * the i2cBootBuffer function.
 *
 * @param offset Offset from the start of the boot EEPROM to start reading
 * @param buf Buffer address to receive the boot EEPROM data
 * @param size Number of bytes to read
 * @returns 0 on success or -1 on failure.
 */
int readFromBootEeprom(uint32_t offset, void *buf, size_t size);

/**
 * @brief Write to the boot EEPROM
 *
 * @details Use this function to write to the boot EEPROM. 
 *
 * @details This function
 * requires the boot i2c COG driver so it must either already be loaded
 * or the memory space used by it cannot have been overwritten by calling
 * the i2cBootBuffer function.
 *
 * @param offset Offset from the start of the boot EEPROM to start writing
 * @param buf Buffer address containing the data to write
 * @param size Number of bytes to write
 * @returns 0 on success or -1 on failure.
 */
int writeToBootEeprom(uint32_t offset, void *buf, size_t size);

#endif

