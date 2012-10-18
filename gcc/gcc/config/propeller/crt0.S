#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
# if defined(DEBUG_KERNEL)
#  include "crt0_xmm_debug.s"
# else
#  include "crt0_xmm.s"
# endif
#elif defined(__PROPELLER_CMM__)
#  include "crt0_cmm.s"
#elif defined(__PROPELLER_LMM__)
# if defined(DEBUG_KERNEL)
#  include "crt0_lmm_debug.s"
# else
#  include "crt0_lmm.s"
# endif
#elif defined(__PROPELLER_COG__)
	/* NOTE: we actually build the real crt0_cog.o from the library,
	   so this probably does not need to be here
	*/
#include "crt0_cog.s"
#else
#error "unknown propeller configuration"
#endif

#if defined(DEBUG_KERNEL)
	/* on exit send a message to the debugger */
	.text
	.global __ExitHook
__ExitHook
	mov	ch, #0xff
	call	#txbyte
	mov	ch, #0
	call	#txbyte
	mov	ch, r0
	call	#txbyte
	mov	lr, pc
#endif
