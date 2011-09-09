/*
 * code to toggle pins
 * this is the cog C version of the code; it runs in a separate
 * cog from the LMM kernel
 */

#include "cog.h"
#include "toggle.h"

/*
 * our local variables (placed in cog memory) for speed
 */
static _COGMEM unsigned int waitdelay;
static _COGMEM unsigned int pins = 0x3fffffff;  /* all pins */
static _COGMEM unsigned int nextcnt;

_NATIVE
void main (volatile struct toggle_mailbox *m)
{
  /* get a half second delay from parameters */
  _DIRA = pins;

  /* figure out how long to wait the first time */
  nextcnt = _CNT + m->wait_time;

  /* loop forever, updating the wait time from the mailbox */
  for(;;) {
    waitdelay = m->wait_time;
    _OUTA ^= pins;
    nextcnt = __builtin_waitcnt(nextcnt, waitdelay);
  }
}
