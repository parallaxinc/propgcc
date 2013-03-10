#if !defined(__PROPELLER2__)

#include <propeller.h>
#include "cogload.h"
#include "i2c.h"

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define EEPROM_ADDR     0xa0

#define BUS_FREQ        100000
#define SCL_PIN         28
#define SDA_PIN         29

/* we need to reference this symbol to get the driver linked */
extern int _i2c_driver_boot;
int _i2c_driver_boot_loaded = (int)&_i2c_driver_boot;

extern unsigned int _load_start_i2c_driver_boot_cog[];

static int cog_i2cBootClose(I2C *dev);

static I2C_OPS cog_i2c_ops = {
    cog_i2cBootClose,
    cog_i2cRead,
    cog_i2cWrite
};

static I2C_COGDRIVER _boot_i2c_data = {
    .i2c.ops = &cog_i2c_ops
};

I2C *_boot_i2c = NULL;

I2C *i2cBootOpen(void)
{
    if (!_boot_i2c) {
        I2C_INIT init;
        
        if (!_i2c_driver_boot_loaded)
            return NULL;
        
        init.scl = SCL_PIN;
        init.sda = SDA_PIN;
        init.ticks_per_cycle = CLKFREQ / BUS_FREQ;
        init.mailbox = &_boot_i2c_data.mailbox;
        
        _boot_i2c_data.mailbox.cmd = I2C_CMD_INIT;
        
        if ((_boot_i2c_data.cog = cognew(_load_start_i2c_driver_boot_cog, &init)) < 0)
            return NULL;
        
        while (_boot_i2c_data.mailbox.cmd != I2C_CMD_IDLE)
            ;
            
        _boot_i2c = (I2C *)&_boot_i2c_data;
    }
    
    return _boot_i2c;
}

static int cog_i2cBootClose(I2C *dev)
{
    cogstop(_boot_i2c_data.cog);
    return 0;
}

void *i2cBootBuffer(void)
{
    _i2c_driver_boot_loaded = 0;
    return (void *)_load_start_i2c_driver_boot_cog;
}

int cognewFromBootEeprom(void *code, size_t codeSize, void *param)
{
    return coginitFromBootEeprom(0x8, code, codeSize, param);
}

int coginitFromBootEeprom(int id, void *code, size_t codeSize, void *param)
{
    uint16_t addr = (uint32_t)code - 0xc0000000 + 0x8000;
    uint8_t cmd[2] = { addr >> 8, addr };
    void *cogbuf;
    
    /* get the address of the boot i2c driver */
    cogbuf = (void *)_load_start_i2c_driver_boot_cog;
    
    /* open the boot i2c bus if it isn't already open */
    if (!_boot_i2c && !i2cBootOpen())
        return -1;
    
    /* mark the boot i2c driver as corrupted so we don't attempt to load it again */
    _i2c_driver_boot_loaded = 0;

    if (i2cWrite(_boot_i2c, EEPROM_ADDR, cmd, 2, FALSE) != 0)
        return -1;
        
    if (i2cRead(_boot_i2c, EEPROM_ADDR, cogbuf, codeSize, TRUE) != 0)
        return -1;
        
    return coginit(id, cogbuf, param);
}

#endif
