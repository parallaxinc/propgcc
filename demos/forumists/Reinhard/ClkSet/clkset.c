/*
 * Variation of Reinhard's program that demonstrated a bug.
 * set clock with : clkset(mode, frequency)
 * get clock info : CLKMODE and CLKFREQ
 *
 * program output change effects visible on o-scope only.
 *
 */
#include <propeller.h>

#define P0 (1<<15)
#define RCFAST 0
#define RCSLOW 1

int main ()
{
    int x = 0;
    int change = 6;
    DIRA |= P0;
    clkset(RCFAST,12000000); 
    for(;;)
    {
        OUTA ^= P0;
        if(x == change)
        {
            clkset(CLKMODE | RCSLOW, 20000);
        }
        if(x == change+1)
        {
            clkset(CLKMODE & ~RCSLOW, 12000000);
            x = 0;
        }
        x++;
    }
    return 0;
}

