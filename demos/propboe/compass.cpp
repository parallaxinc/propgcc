#include <stdio.h>
#include "Term.h"
#include "I2C.h"

#define COMPASS_ADDR    0x1e

void CompassInit(I2C &dev);
void CompassRead(I2C &dev, int &x, int &y, int &z);

int main(void)
{
    SerialTerm serial(stdout);
    I2C bus(1, 0);

    CompassInit(bus);
    
    for (;;) {
        int x, y, z;
        
        CompassRead(bus, x, y, z);
        serial.str("x ");
        serial.dec(x);
        serial.str(", y ");
        serial.dec(y);
        serial.str(", z ");
        serial.dec(z);
        serial.str("\n");
    
        pause(2000);
    }

    return 0;
}

void CompassInit(I2C &bus)
{
    /* set to continuous mode */
    bus.begin(COMPASS_ADDR);
    bus.send(0x02);
    bus.send(0x00);
    bus.end();
}

void CompassRead(I2C &bus, int &x, int &y, int &z)
{
    uint8_t data[6];
    int16_t x16, y16, z16;
    
    /* select the data registers */
    bus.begin(COMPASS_ADDR);
    bus.send(0x03);
    bus.end();
    
    /* read the data registers */
    bus.request(COMPASS_ADDR, 6, data);

    /* assemble the return values */
    x16 = (data[0] << 8) | data[1];
    z16 = (data[2] << 8) | data[3];
    y16 = (data[4] << 8) | data[5];
    
    /* return the signed values */
    x = x16;
    y = y16;
    z = z16;
}
