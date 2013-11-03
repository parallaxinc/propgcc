#if !defined(__PROPELLER2__)
/*
 * very simple COG program to keep the 64 bit _default_ticks variable
 * up to date
 */
#include <propeller.h>
#include <sys/rtc.h>

__asm__(
"    .section .cogrtcupdate,\"ax\"\n"
"    .compress off\n"
"L_main\n"

"    rdlong oldlo, default_ticks_ptr\n"
"    mov    newlo, CNT\n"
"    cmp    newlo,oldlo wc\n"
"    add    default_ticks_ptr,#4\n"
"    rdlong newhi, default_ticks_ptr\n" 
"    addx   newhi,#0\n"  /* adds in the carry set above */
"    sub    default_ticks_ptr,#4\n"

/* the sequence here makes sure to write newlo,newhi in that
 * order and in the fewest possible hub windows; if all readers
 * of default_ticks also read lo,hi in the fewest possible
 * hub cycles, then all users will
 * see consistent values
 */
"    wrlong newlo, default_ticks_ptr\n"
"    add    default_ticks_ptr,#4\n"
"    wrlong newhi, default_ticks_ptr\n"
"    sub    default_ticks_ptr,#4\n"
"    jmp    #L_main\n"
"newlo long 0\n"
"newhi long 0\n"
"oldlo long 0\n"
"default_ticks_ptr long __default_ticks\n"
"    .compress default\n"
	);

void
_rtc_start_timekeeping_cog(void)
{
  extern unsigned int _load_start_cogrtcupdate[];

  if (_default_ticks_updated)
    return;  /* someone is already updating the time */

  _default_ticks_updated = 1;

#if defined(__PROPELLER_XMMC__) || defined(__PROPELLER_XMM__)
    unsigned int *buffer;

    // allocate a buffer in hub memory for the cog to start from
    buffer = __builtin_alloca(2048);
    memcpy(buffer, _load_start_cogrtcupdate, 2048);
    cognew(buffer, 0);
#else
    cognew(_load_start_cogrtcupdate, 0);
#endif
}
#endif
