#include <propeller.h>
#include "i2c.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define EEPROM_ADDR 0xa0

static uint8_t cogbuf[496 * sizeof(uint32_t)];

int cognewFromEeprom(I2C *bus, void *fw, void *parptr)
{
    uint16_t addr = (uint32_t)fw - 0xc0000000 + 0x8000;
    uint8_t cmd[2] = { addr >> 8, addr };
    //void *cogbuf = i2cCogBuffer();
    int id;
    
    if (i2cWrite(bus, EEPROM_ADDR, cmd, 2, FALSE) != 0)
        return -1;
        
    if (i2cRead(bus, EEPROM_ADDR, cogbuf, 496 * sizeof(uint32_t), TRUE) != 0)
        return -1;
        
    if ((id = cognew(cogbuf, parptr)) < 0)
        return -1;
        
    return id;
}
