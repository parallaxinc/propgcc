#if defined(__PROPELLER_XMM__) || defined(__PROPELLER_XMMC__)
#include "crt0_xmm.s"
#elif defined(__PROPELLER_LMM__)
#include "crt0_lmm.s"
#elif defined(__PROPELLER_COG__)
#include "crt0_cog.s"
#else
#error "unknown propeller configuration"
#endif
