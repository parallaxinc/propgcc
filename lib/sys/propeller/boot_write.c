#include <propeller.h>
#include "cogload.h"
#include "i2c.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define EEPROM_ADDR 	0xa0

extern I2C *_boot_i2c;

int writeToBootEeprom(uint32_t offset, void *buf, size_t size)
{
    uint8_t cmd[2] = { offset >> 8, offset };
    
    /* open the boot i2c bus if it isn't already open */
    if (!_boot_i2c && !i2cBootOpen())
    	return -1;
    
    if (i2cWrite(_boot_i2c, EEPROM_ADDR, cmd, 2, FALSE) != 0)
        return -1;
        
    return i2cWrite(_boot_i2c, EEPROM_ADDR, buf, size, TRUE);
}
