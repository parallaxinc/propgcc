/*
 * very simple COG program to keep the 64 bit _default_ticks variable
 * up to date
 */
#include <propeller.h>
#include <sys/rtc.h>

__asm__(
"    .section .cogrtcupdate,\"ax\"\n"
"L_main\n"
"    rdlong lastlo, default_ticks_ptr\n"
"    mov    now, CNT\n"
"    wrlong now, default_ticks_ptr\n"
"    cmp    now,lastlo wc\n"
" IF_NC jmp #L_main\n"
"    add    default_ticks_ptr,#4\n"
"    rdlong lasthi, default_ticks_ptr\n" 
"    add    lasthi,#1\n"
"    wrlong lasthi, default_ticks_ptr\n"
"    sub    default_ticks_ptr,#4\n"
"    jmp    #L_main\n"
"lastlo long 0\n"
"lasthi long 0\n"
"now    long 0\n"
"default_ticks_ptr long __default_ticks\n"
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
