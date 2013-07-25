#include <stdio.h>
#include <i2c.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define EEPROM_ADDR     0xa0

typedef struct {
    I2C *dev;
    int address;
} EEPROM;

void eepromInit(EEPROM *eeprom, I2C *dev, int address);
int eepromRead(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count);
int eepromWrite(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count);

int main(void)
{
    I2C *dev;
    uint8_t buf[128];
    EEPROM eeprom;
    int n;
    
    if ((dev = i2cBootOpen()) == NULL) {
        printf("i2cBootOpen failed\n");
        return 1;
    }
    
    eepromInit(&eeprom, dev, EEPROM_ADDR);
    
    if (eepromRead(&eeprom, 0x8000, buf, sizeof(buf)) != 0) {
        printf("eepromRead failed\n");
        return 1;
    }
    
    if (strncmp(buf, "Testing", 7) == 0)
        n = atoi(&buf[8]) + 1;
    else
        n = 0;
        
    sprintf(buf, "Testing %d\n", n);
    fputs(buf, stdout);

    if (eepromWrite(&eeprom, 0x8000, buf, sizeof(buf)) != 0) {
        printf("eepromWrite failed\n");
        return 1;
    }    
    
    return 0;
}

void eepromInit(EEPROM *eeprom, I2C *dev, int address)
{
    eeprom->dev = dev;
    eeprom->address = address;
}

int eepromRead(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count)
{
    uint8_t buf[2] = { address >> 8, address };
    
    if (i2cWrite(eeprom->dev, eeprom->address, buf, 2, FALSE) != 0)
        return -1;
        
    if (i2cRead(eeprom->dev, eeprom->address, buffer, count, TRUE) != 0)
        return -1;
        
    return 0;
}

int eepromWrite(EEPROM *eeprom, uint32_t address, uint8_t *buffer, int count)
{
    uint8_t buf[2] = { address >> 8, address };
    
    if (i2cWrite(eeprom->dev, eeprom->address, buf, 2, FALSE) != 0)
        return -1;
        
    if (i2cWriteMore(eeprom->dev, buffer, count, TRUE) != 0)
        return -1;
        
    return 0;
}
        
