#include <stdio.h>
#include <driver.h>
#include "boe.h"

extern _Driver _FullDuplexSerialDriver;

_Driver *_driverlist[] = {
  &_FullDuplexSerialDriver,
  NULL
};

int main(void)
{
    FILE *fp;
    
    if (!(fp = fopen("FDS:9600,1,0", "r+")))
        printf("can't open serial port\n");
    else {
        for (;;) {
            fprintf(fp, "Hello, World!\n");
            pause(500);
        }
    }
    
    return 0;
}
