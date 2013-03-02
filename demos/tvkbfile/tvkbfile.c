#include <stdio.h>
#include <propeller.h>
#include <sys/sd.h>
#include "keyboard.h"

/* This is a list of all drivers we can use in the
 * program. The default _InitIO function opens stdin,
 * stdout, and stderr based on the first driver in
 * the list (the serial driver, for us)
 */
extern _Driver _SimpleSerialDriver;
extern _Driver TvDriver;
extern _Driver _FileDriver;

_Driver *_driverlist[] = {
    &TvDriver,
    &_FileDriver,
    NULL
};

int main(void)
{
    char *filename = "abe.txt";
    char *msg = "Everthing you read on the internet is true. - Abraham Lincoln\n";
    char rdmsg[256];
    FILE *fp;

    setbuf(stdout, 0);
    printf("TV Keybd File Demo ...\n\n");

    fp = fopen(filename, "w");
    if(fp > 0) {
        fprintf(fp,msg);
        fclose(fp);
    }
    else {
        printf("Can't open '%s' for write.\n", filename);
    }

    fp = fopen(filename, "r");
    if(fp > 0) {
        fread(rdmsg,1,256,fp);
        fclose(fp);
        printf("%s",rdmsg);
    }
    else {
        printf("Can't open '%s' for read.\n", filename);
    }

    printf("\nPress any key ...\n");
    while(1) {
        putchar(getchar());
    }
    return 0;
}