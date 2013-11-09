#include <stdio.h>
#include <eeprom.h>

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define EEPROM_ADDR     0xa0

int main(void)
{
    EEPROM_COGDRIVER state;
    EEPROM *eeprom;
    uint8_t buf[128];
    int n;
    
    if ((eeprom = eepromOpen(&state, 28, 29, 400000, EEPROM_ADDR)) == NULL) {
        printf("eepromOpen failed\n");
        return 1;
    }
    
    if (eepromRead(eeprom, 0x8000, buf, sizeof(buf)) != 0) {
        printf("eepromRead failed\n");
        return 1;
    }
    
    if (strncmp(buf, "Testing", 7) == 0)
        n = atoi(&buf[8]) + 1;
    else
        n = 0;
        
    sprintf(buf, "Testing %d\n", n);
    fputs(buf, stdout);

    if (eepromWrite(eeprom, 0x8000, buf, sizeof(buf)) != 0) {
        printf("eepromWrite failed\n");
        return 1;
    }    
    
    return 0;
}
