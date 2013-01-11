#if defined(__PROPELLER2__)
#undef DEBUG_KERNEL
#endif

#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
# if defined(DEBUG_KERNEL)
#  include "crt0_xmm_debug.s"
# else
#  include "crt0_xmm.s"
# endif
#elif defined(__PROPELLER_CMM__)
#  if defined(__PROPELLER2__)
#    include "crt0_cmm2.s"
#  else
#    include "crt0_cmm.s"
#  endif
#elif defined(__PROPELLER_LMM__)
# if defined(DEBUG_KERNEL)
#  if defined(__PROPELLER2__)
#    include "crt0_lmm2_debug.s"
#  else
#    include "crt0_lmm_debug.s"
#  endif
# else
#  if defined(__PROPELLER2__)
#    include "crt0_lmm2.s"
#  else
#    include "crt0_lmm.s"
#  endif
# endif
#elif defined(__PROPELLER_COG__)
	/* NOTE: we actually build the real crt0_cog.o from the library,
	   so this probably does not need to be here
	*/
#include "crt0_cog.s"
# if defined(__PROPELLER2__)
#   include "crt0_cog2.s"
# else
#   include "crt0_cog.s"
# endif
#else
#error "unknown propeller configuration"
#endif
