#include <cog.h>

/* globals that the loader may change; these represent the default
 * pins to use
 */
extern unsigned int _rxpin;
extern unsigned int _txpin;
extern unsigned int _baud;

/* send a break on the default transmit pin */

void _serial_break(void)
{
#ifdef __PROPELLER2__
#else
  int delay = _clkfreq/2;
  int waitcycles = _CNT + delay;
  int txpin = _txpin;

  _DIRA |= (1<<txpin);
  _OUTA |= (1<<txpin);

  /* sleep a bit to let things drain */
  waitcycles = __builtin_propeller_waitcnt(waitcycles, delay);

  /* send a break */
  _OUTA = 0;
  __builtin_propeller_waitcnt(waitcycles, delay);
#endif
}
