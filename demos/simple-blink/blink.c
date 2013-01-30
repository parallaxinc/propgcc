#include <propeller.h>

void main(void)
{
  unsigned int wait_time = CLKFREQ / 2;
  unsigned int nextcnt = getcnt() + wait_time;
#ifdef __PROPELLER2__
  unsigned int pin = 32;	// de2-115
#else
  unsigned int pin = 16;	// quickstart
#endif

  for (;;) {
    togglepin(pin);
    nextcnt = waitcnt2(nextcnt, wait_time);
  }
}
