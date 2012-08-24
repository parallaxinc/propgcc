#include <stdio.h>
#include <i2c.h>
#include "term_serial.h"
#include "misc.h"

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define COMPASS_ADDR    0x1e

//#define USE_SIMPLE_I2C_DRIVER

void CompassInit(I2C *dev);
void CompassRead(I2C *dev, int *px, int *py, int *pz);

TERM *term;

int main(void)
{
    TERM_SERIAL serial;
#ifdef USE_SIMPLE_I2C_DRIVER
    I2C_SIMPLE i2c;
#else
    I2C_COGDRIVER i2c;
#endif
    I2C *bus;
    
    term = serialTermStart(&serial, stdout);
    
#ifdef USE_SIMPLE_I2C_DRIVER
    bus = simple_i2cOpen(&i2c, 1, 0);
#else
    bus = i2cOpen(&i2c, 1, 0, 400000);
#endif

    CompassInit(bus);
    
    for (;;) {
        int x, y, z;
        
        CompassRead(bus, &x, &y, &z);
        termStr(term, "x ");
        termDec(term, x);
        termStr(term, ", y ");
        termDec(term, y);
        termStr(term, ", z ");
        termDec(term, z);
        termStr(term, "\n");
    
        pause(2000);
    }

    return 0;
}

uint8_t continuous_mode[] = { 0x02, 0x00 };

void CompassInit(I2C *bus)
{
    /* set to continuous mode */
    if (i2cWrite(bus, COMPASS_ADDR, continuous_mode, sizeof(continuous_mode), TRUE) != 0)
        termStr(term, "Setting continuous mode failed\n");
}

uint8_t read_data_registers[] = { 0x03 };

void CompassRead(I2C *bus, int *px, int *py, int *pz)
{
    int16_t x16, y16, z16;
    uint8_t data[6];
    
    /* select the data registers */
    if (i2cWrite(bus, COMPASS_ADDR, read_data_registers, sizeof(read_data_registers), TRUE) != 0)
    	termStr(term, "Write failed\n");
    
    /* read the data registers */
    if (i2cRead(bus, COMPASS_ADDR, data, sizeof(data), TRUE) != 0)
        termStr(term, "Read failed\n");

    /* assemble the return values */
    x16 = (data[0] << 8) | data[1];
    z16 = (data[2] << 8) | data[3];
    y16 = (data[4] << 8) | data[5];
    
    /* return the signed values */
    *px = x16;
    *py = y16;
    *pz = z16;
}
