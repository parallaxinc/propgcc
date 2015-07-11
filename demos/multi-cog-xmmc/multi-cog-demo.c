/* multi-cog-demo.c - a simple multi-cog program that can be built using the lmm, cmm, or xmm memory model */

/*
    One thing that I noticed when writing this is that there really needs to be a version of cogstart that
    takes a COG number like coginit does. That way you could restart COG 0.
*/

/* compile using one of these commands:

    propeller-elf-gcc -Os -mlmm -o multi-cog-lmm.elf multi-cog-demo.elf
    propeller-elf-gcc -Os -mcmm -o multi-cog-cmm.elf multi-cog-demo.elf
    propeller-elf-gcc -Os -mxmmc -o multi-cog-xmmc.elf multi-cog-demo.elf
*/

#include <propeller.h>

#define BASE_PIN    16

#ifdef __PROPELLER_USE_XMM__
#define MAX_COGS    7   // in the XMM memory model one COG is used to run the external memory driver
#else
#define MAX_COGS    8   // in the LMM and CMM memory models there is no external memory driver so all 8 COGs can be used
#endif

/* thread parameter structure */
typedef struct {
    volatile int done;
    int pin;
    int delay;
} ToggleInfo;

/* stacks for each COG. The EXTRA_STACK_LONGS define guarantees that the stacks are big enough no matter which memory model is used */
static long stacks[8][16 + EXTRA_STACK_LONGS];

/* thread function */
static void toggle(void *par);

int main(void)
{
    ToggleInfo info;
    int i;
    
    /* start the first MAX_COGS - 1 COGs */
    for (i = 0; i < MAX_COGS - 1; ++i) {
        info.done = 0;
        info.pin = BASE_PIN + i;
        info.delay = CLKFREQ >> i;
        cogstart(toggle, &info, &stacks[i], sizeof(stacks[i]));
        while (!info.done)
            ;
    }
    
    /* start the last COG (need cogstartn to avoid this special case) */
    info.pin = BASE_PIN + MAX_COGS - 1;
    info.delay = CLKFREQ >> (MAX_COGS - 1);
    toggle(&info);
    
    return 0;
}

static void toggle(void *par)
{
    ToggleInfo *info = (ToggleInfo *)par;
    uint32_t mask = 1 << info->pin;
    int delay = info->delay;
    
    info->done = 1;
    
    DIRA |= mask;
    
    while (1) {
        OUTA ^= mask;
        waitcnt(delay + CNT);
    }
}