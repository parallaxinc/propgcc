#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
# if defined(DEBUG_KERNEL)
#  include "crt0_xmm_debug.s"
# else
#  include "crt0_xmm.s"
# endif
#elif defined(__PROPELLER_LMM__)
# if defined(DEBUG_KERNEL)
#  include "crt0_lmm_debug.s"
# else
#  include "crt0_lmm.s"
# endif
#elif defined(__PROPELLER_COG__)
#include "crt0_cog.s"
#else
#error "unknown propeller configuration"
#endif
