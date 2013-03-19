//
// Test program for Tiny I/O stdio-replacement library
//

#include <propeller.h>
#include <sys/thread.h>
#include <stdlib.h>
//#include <stdio.h>   /* You can include this instead of tinyio.h, but the put*/get* stuff won't work */
#include <tinyio.h>    /* Be sure to include only one of stdio.h or tinyio.h */

//#define printf __simple_printf

//
// This block is not necessary, unless:
//    you have different rx/tx pins and/or baud rate
unsigned int _rxpin = 31;
unsigned int _txpin = 30;
unsigned int _baud = 115200;  // Lower if your clock is less than 80Mhz

long threadStacks[2][256];
_thread_state_t tls[2];
#define threadStack(n) &threadStacks[n][sizeof(threadStacks[0])]

void testThread(void* pid);

int main()
{
    printf("This works %d%%\n", 100);
    printf("Hello World!\n");
    printf("Norm  nums: %d %u %x %c %s%s", -1, -2, 0xabcdefab, 'X', "x", "\n");

    // Note: these don't work with __simple_printf!
    printf("BFill nums: %4d %12u %10x %3c %3s%s", -1, -2, 0xabcdefab, 'X', "x", "\n");
    printf("LFill nums: %04d %012u %010x %3c %3s%s", -1, -2, 0xabcdefab, 'X', "x", "\n");
#pragma GCC diagnostic ignored "-Wformat"
    // The underlying library actively ignores the zeros -- we want to test it!
    printf("RFill nums: %-04d %-012u %-010x %-3c %-3s%s", -1, -2, 0xabcdefab, 'X', "x", "\n");
    // The underlying library detects and handles this error - we want to test it!
    printf("%");
#pragma GCC diagnostic warning "-Wformat"

#ifdef __TINY_IO_H
    putstr("Put*  nums: ");
    putdec(-1);
    putchar(' ');
    putudec(-1);
    putchar(' ');
    puthex(0xabcdefab);
    putchar(' ');
    putchar('X');
    putchar(' ');
    putstr("x");
    putchar(' ');
    putfnum(0xabcdefab, 16, 0, 10, '0');
    putchar('\n');

    putlhex(-10000000000L);
    putchar(' ');
    putldec(-10000000000L);
    putchar(' ');
    putlfnum(0xabcdefabcdef, 16, 0, 16, '0');
    putchar('\n');
#endif

    char buf[80];
    sprintf(buf, "Norm  nums: %d %u %x %c %s", -1, -2, 0xabcdefab, 'X', "x");
    puts(buf);
    int d;
    unsigned u, x;
    char c;
    char s[20];
    int n = sscanf(buf, "Norm  nums: %d %u %x %c %s\n", &d, &u, &x, &c, s);
    printf("Scan  nums: %d %u %x %c %s (%d scanned)\n", d, u, x, c, s, n);

    printf("\nGimme a character: ");
    char ch = getchar();
    printf("\nYou typed: ");
    putchar(ch);
    putchar('\n');

    int age;
    do {
        printf("\nHow old are you? ");
        scanf("%u", &age);
    } while (!age);
    printf("In ten years, you'll be: %d\n\n", age + 10);

    sprintf(buf, "BFill nums: %4d %12u %10x %3c %3s", -1, -2, 0xabcdefab, 'X', "x");
    puts(buf);
    n = sscanf(buf, "BFill nums: %4d %12u %10x %3c %3s\n",&d, &u, &x, &c, s);
    printf("Scan  nums: %d %u %x %c %s (%d scanned)\n", d, u, x, c, s, n);

    sprintf(buf, "LFill nums: %04d %012u %010x %3c %3s", -1, -2, 0xabcdefab, 'X', "x");
    puts(buf);
    n = sscanf(buf, "LFill nums: %04d %012u %010x %3c %3s\n",&d, &u, &x, &c, s);
    printf("Scan  nums: %d %u %x %c %s (%d scanned)\n", d, u, x, c, s, n);

#ifdef __TINY_IO_H
    printf("\nEnter a string up to 4 characters: ");
    safe_gets(s, 5);
    printf("You entered: %s\n", s);

    printf("\nEnter a decimal: ");
    d = getdec();
    printf("You entered: %d\n", d);

    printf("\nEnter an unsigned decimal: ");
    u = getudec();
    printf("You entered: %d\n", u);

    printf("\nEnter a hex number: ");
    u = gethex();
    printf("You entered: %x\n", u);

    printf("\nEnter a 1-4 digit number: ");
    d = getfnum(10, 1, 4);
    printf("You entered: %d\n", (int)d);

    long long ld;
    unsigned long long lu, lx;

    printf("\nEnter a long long decimal: ");
    ld = getldec();
    printf("You entered: "); putldec(ld); putchar('\n');

    printf("\nEnter an unsigned long long decimal: ");
    lu = getludec();
    printf("You entered: "); putludec(lu); putchar('\n');

    printf("\nEnter a long long hex number: ");
    lx = getlhex();
    printf("You entered: "); putlhex(lx); putchar('\n');

    printf("\nEnter a 1-12 digit long long number: ");
    ld = getlfnum(10, 1, 12);
    printf("You entered: "); putldec(ld); putchar('\n');
#endif

    _serialLock = 1;
    printf("\n\nMultithreaded tests.\n"
           "Note this will be quite messed up because multiple cogs\n"
           "will be doing I/O on a character-by-character basis...\n");
    _start_cog_thread(threadStack(0), &testThread, (void*)1, &tls[0]);
    _start_cog_thread(threadStack(1), &testThread, (void*)2, &tls[1]);
    testThread(0);

    printf("\nBye!\n");

    return 0;
}

void testThread(void* pid)
{
    int id = (int)pid;

    while (1)
    {
        printf("\nIn cog %d, enter a decimal (enter %d to exit): ", id, id);
        int d = getdec();
        printf("From cog %d, you entered: %d\n", id, d);
        if (d == id)
        {
            printf("\nCog %d exiting.  Bye!\n", id);
            exit(0);
        }
    }
}
