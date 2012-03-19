#include <stdio.h>
#include "boe.h"

#define COMPASS_ADDR    0x1e

void CompassInit(I2C *dev);
void CompassRead(I2C *dev, int *px, int *py, int *pz);

int main(void)
{
    I2C dev;
    
    i2cInit(&dev, 1, 0);
    
    CompassInit(&dev);

    for (;;) {
        int x, y, z;
        
        CompassRead(&dev, &x, &y, &z);
        printf("x %d, y %d, z %d\n", x, y, z);
    
        pause(2000);
    }

    return 0;
}

void CompassInit(I2C *dev)
{
    /* set to continuous mode */
    i2cBegin(dev, COMPASS_ADDR);
    i2cSend(dev, 0x02);
    i2cSend(dev, 0x00);
    i2cEnd(dev);
}

void CompassRead(I2C *dev, int *px, int *py, int *pz)
{
    uint8_t data[6];
    int16_t x, y, z;
    int i;
    
    /* select the data registers */
    i2cBegin(dev, COMPASS_ADDR);
    i2cSend(dev, 0x03);
    i2cEnd(dev);
    
    /* prepare to read the data registers */
    i2cRequest(dev, COMPASS_ADDR, 6);
    
    /* read the data registers */
    for (i = 0; i < 6; ++i)
        data[i] = i2cReceive(dev);
        
    /* assemble the return values */
    x = (data[0] << 8) | data[1];
    z = (data[2] << 8) | data[3];
    y = (data[4] << 8) | data[5];
    
    /* return the signed values */
    *px = x;
    *py = y;
    *pz = z;
}
