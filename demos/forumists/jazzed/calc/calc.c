#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

const char *cmdchars = "+, -, *, or /";


// size of buffer for getting numbers
#define NUMSIZE 12


float getfloat(int num)
{
    static char buff[NUMSIZE];    
    printf("Enter %s number: ", (num==1) ? "first ":"second");
    fgets(buff, NUMSIZE, stdin);
    return atof(buff);          // strtod works too, but it is bigger
}


int main(void)
{
    char ch = 0;    // assign to supress uninitialized warnings
    double v1 = 2;
    double v2 = 3;

    sleep(1);       // give terminal time to start
    setbuf(stdout, 0);
    printf("%f\n", v1+v2);


    printf("Very Simple Propeller Calculator Demo\n");
    printf("Commands %s then number1 number2.", cmdchars);
    for(;;)
    {
        printf("\nCommand? ");

        /* handle input */
        ch = getchar();
        switch(ch)
        {
            case 'q' :
                return 0;
            case '+' :  // fall through
            case '-' :  // fall through
            case '*' :  // fall through
            case '/' :  // fall through
                getchar();  // clobber newline
                v1 = getfloat(1);
                v2 = getfloat(2);
                break;
            case '\r' : // fall through
            case '\n' : // fall through
                ch = 0; // not a valid command
                break;  // ignore these
            default :
                ch = 0; // not a valid command
                printf("Invalid command. Please enter one of these first: %s\n", cmdchars);
                printf("Then enter 2 numbers for the operation.\n");
                break;
        }

        /* print output */
        if(ch)
        {
            printf("%f %c %f = ",v1,ch,v2);
            switch(ch)
            {
                case '+' :
                    printf("%f",v1+v2);
                    break;
                case '-' :
                    printf("%f",v1-v2);
                    break;
                case '*' :
                    printf("%f",v1*v2);
                    break;
                case '/' :
                    printf("%f",v1/v2);
                    break;
                default :
                    break;
            }
            printf("\n");
        }
    }
    return 0;
}

