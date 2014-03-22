#include <stdio.h>
#include "eeprom.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

EEPROM *eepromOpen(EEPROM_COGDRIVER *eeprom, int scl, int sda, int freq, int address)
{
    I2C *dev;
    if (!(dev = i2cOpen(&eeprom->dev, scl, sda, freq)))
        return NULL;
    eepromInit(&eeprom->state, dev, address);
    return (EEPROM *)eeprom;
}

EEPROM *simple_eepromOpen(EEPROM_SIMPLE *eeprom, int scl, int sda, int address)
{
    I2C *dev;
    if (!(dev = simple_i2cOpen(&eeprom->dev, scl, sda)))
        return NULL;
    eepromInit(&eeprom->state, dev, address);
    return (EEPROM *)eeprom;
}

EEPROM *eepromBootOpen(EEPROM_BOOT *eeprom, int address)
{
    I2C *dev;
    if (!(dev = i2cBootOpen()))
        return NULL;
    eepromInit(&eeprom->state, dev, address);
    return (EEPROM *)eeprom;
}

void eepromInit(EEPROM *eeprom, I2C *dev, int address)
{
    eeprom->dev = dev;
    eeprom->address = address;
}

int eepromRead(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count)
{
    uint8_t buf[2] = { address >> 8, address };
    
    /* write the i2c header and the eeprom address */
    if (i2cWrite(eeprom->dev, eeprom->address, buf, 2, FALSE) != 0)
        return -1;
        
    /* read the eeprom data */
    if (i2cRead(eeprom->dev, eeprom->address, buffer, count, TRUE) != 0)
        return -1;
        
    return 0;
}

int eepromWrite(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count)
{
    uint8_t buf[2] = { address >> 8, address };
    
    /* write the i2c header and the eeprom address */
    if (i2cWrite(eeprom->dev, eeprom->address, buf, 2, FALSE) != 0)
        return -1;
        
    /* write the eeprom data */
    if (i2cWriteMore(eeprom->dev, buffer, count, TRUE) != 0)
        return -1;
        
    /* wait for the write to complete (maybe should have a timeout?) */
    while (i2cWrite(eeprom->dev, eeprom->address, NULL, 0, TRUE) != 0)
        ;
        
    return 0;
}

int eepromClose(EEPROM *eeprom)
{
    if (!eeprom->dev)
        return -1;
    i2cClose(eeprom->dev);
    eeprom->dev = NULL;
    return 0;
}
