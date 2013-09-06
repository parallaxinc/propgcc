#include <stdio.h>
#include <propeller.h>

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

#if 1    
    OUTA &= ~(1<<15);
    DIRA |= 1<<15;
    while(1);
#endif

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
    while(1);
}
