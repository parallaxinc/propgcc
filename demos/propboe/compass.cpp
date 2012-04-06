#include <stdio.h>
#include "Term.h"
#include "I2C.h"

#define COMPASS_ADDR    0x1e

void CompassInit(I2C &dev);
void CompassRead(I2C &dev, int &x, int &y, int &z);

SerialTerm *ser;

int main(void)
{
    SerialTerm serial(stdout);
    I2C bus(1, 0, 250000);

    ser = &serial;
    
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

uint8_t continuous_mode[] = { 0x02, 0x00 };

void CompassInit(I2C &bus)
{
    /* set to continuous mode */
    if (bus.send(COMPASS_ADDR, continuous_mode, sizeof(continuous_mode)) != 0)
        ser->str("Setting continuous mode failed\n");
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
    if (bus.request(COMPASS_ADDR, data, sizeof(data)) != 0)
        ser->str("Read failed\n");

    /* assemble the return values */
    x16 = (data[0] << 8) | data[1];
    z16 = (data[2] << 8) | data[3];
    y16 = (data[4] << 8) | data[5];
    
    /* return the signed values */
    x = x16;
    y = y16;
    z = z16;
}
