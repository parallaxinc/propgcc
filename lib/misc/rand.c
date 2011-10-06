#include <stdlib.h>

long _rseed = 1;

int rand(void)
{
    long k1;

    /* make sure we don't get stuck at zero */
    if (_rseed == 0) _rseed = 1;

    /* algorithm taken from Dr. Dobbs Journal, November 1985, page 91 */
    k1 = _rseed / 127773L;
    if ((_rseed = 16807L * (_rseed - k1 * 127773L) - k1 * 2836L) < 0L)
        _rseed += 2147483647L;

    /* return a random number between 0 and 2^31-1 */
    return _rseed & 0x7fffffff;
}
