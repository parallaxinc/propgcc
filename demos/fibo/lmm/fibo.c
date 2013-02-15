//#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include "stdio.h"
#include "cog.h"

#define CNT _CNT
#define CLKFREQ _CLKFREQ

unsigned int fibo (unsigned int n)
{
    if (n < 2)
    {
        return (n);
    }
    else
    {
        return fibo(n - 1) + fibo(n - 2);
    }
}

extern unsigned int clock(void);

void main (int argc,  char* argv[])
{
    int n;
    int result;
    unsigned int startTime;
    unsigned int endTime;
    unsigned int executionTime;
    unsigned int rawTime;

    printf("hello, world!\r\n");
    for (n = 0; n <= 26; n++)
    {
        printf("fibo(%02d) = ", n);
        startTime = clock();
        result = fibo(n);
        endTime = clock();
        rawTime = endTime - startTime;
        executionTime = rawTime / (CLKFREQ / 1000);
        printf ("%06d (%05ums) (%u ticks)\n", result, executionTime, rawTime);
    }
}
