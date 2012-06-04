#include <stdio.h>
#include "term_serial.h"
#include "i2c.h"
#include "misc.h"

#define USE_SIMPLE_I2C_DRIVER

#define COMPASS_ADDR    0x1e

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
    bus = simple_i2cInit(&i2c, 1, 0);
#else
    bus = i2cInit(&i2c, 1, 0, 400000);
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
    if (i2cSendBuf(bus, COMPASS_ADDR, continuous_mode, sizeof(continuous_mode)) != 0)
        termStr(term, "Setting continuous mode failed\n");
}

void CompassRead(I2C *bus, int *px, int *py, int *pz)
{
    int16_t x16, y16, z16;
    uint8_t data[6];
    
    /* select the data registers */
    i2cBegin(bus, COMPASS_ADDR);
    i2cAddByte(bus, 0x03);
    i2cSend(bus);
    
    /* read the data registers */
    if (i2cRequestBuf(bus, COMPASS_ADDR, data, sizeof(data)) != 0)
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
