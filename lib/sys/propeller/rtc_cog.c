/*
 * very simple COG program to keep the 64 bit _default_ticks variable
 * up to date
 */
#include <cog.h>

extern union {
  unsigned long long curticks;
  struct {
    unsigned int lo;
    unsigned int hi;
  } s;
} _default_ticks;

extern int _default_ticks_updated;

int
main(void)
{
  unsigned long lasttick, curtick;
  unsigned long sleeptime;
  // update 1024 times per second or so
  unsigned long freq = _clkfreq >> 10;

  curtick = _CNT;
  _default_ticks_updated = 1;  /* take control of _default_ticks */
  _default_ticks.s.hi = 0;
  _default_ticks.s.lo = lasttick;
  sleeptime = curtick + freq;

  for(;;) {
    sleeptime = __builtin_propeller_waitcnt(sleeptime, freq);
    lasttick = curtick;
    curtick = _CNT;
    if (lasttick < curtick) {
      // counter has wrapped around
      _default_ticks.s.hi++;
    }
    _default_ticks.s.lo = curtick;
  }
}
