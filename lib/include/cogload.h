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
 * @brief Start a new Propeller PASM COG from a COG image in EEPROM
 *
 * @details This is use to start a COG with code compiled as PASM, AS, or COG-C
 * when the COG image is in EEPROM.
 * PASM can be any Spin/PASM code that is self-contained. That is, all data
 * for initialization and mailbox use are passed via the par parameter.
 * Changing PASM variables from SPIN code will not work with this method.
 *
 * @details GAS and COG-C programs have similar restrictions.
 * COG-C programs should not use any stack or variables in HUB memory that
 * are not accessed via PAR mailbox or pointers.
 *
 * @param code Address of PASM to load
 * @param size of code in bytes
 * @param param Value of par parameter usually an address
 * @returns COG ID provided by the builtin function or -1 on failure.
 */
int cognewFromBootEeprom(void *code, size_t codeSize, void *param);

I2C *i2cBootOpen(void);
void *i2cBootBuffer(void);
int readFromBootEeprom(uint32_t offset, void *buf, size_t size);
int writeToBootEeprom(uint32_t offset, void *buf, size_t size);

#endif

