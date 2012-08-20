/* k_sinf.c -- float version of k_sin.c
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include "math.h"
#include "math_private.h"

static const float 
half =  5.0000000000e-01,/* 0x3f000000 */
S1  = -1.6666667163e-01, /* 0xbe2aaaab */
S2  =  8.3333337680e-03, /* 0x3c088889 */
S3  = -1.9841270114e-04, /* 0xb9500d01 */
S4  =  2.7557314297e-06, /* 0x3638ef1b */
S5  = -2.5050759689e-08, /* 0xb2d72f34 */
S6  =  1.5896910177e-10, /* 0x2f2ec9d3 */
one =  1.0000000000e+00, /* 0x3f800000 */
C1  =  4.1666667908e-02, /* 0x3d2aaaab */
C2  = -1.3888889225e-03, /* 0xbab60b61 */
C3  =  2.4801587642e-05, /* 0x37d00d01 */
C4  = -2.7557314297e-07, /* 0xb493f27c */
C5  =  2.0875723372e-09, /* 0x310f74f6 */
C6  = -1.1359647598e-11; /* 0xad47d74e */

static void
__kernel_sincosf(float x, float y, int iy, float *sinptr, float *cosptr)
{
	float z,rs,v;
    float a,hz,qx,rc;
	int32_t ix;
	GET_FLOAT_WORD(ix,x);
	ix &= 0x7fffffff;			/* high word of x */
	if(ix<0x32000000) {			/* |x| < 2**-27 */
        if((int)x==0) {
            *sinptr = x;
            *cosptr = one;
        }
    }		/* generate inexact */
	z	=  x*x;
	v	=  z*x;
	rs	=  S2+z*(S3+z*(S4+z*(S5+z*S6)));
	if(iy==0) rs = x+v*(S1+z*rs);
	else      rs = x-((z*(half*y-v*rs)-y)-v*S1);

	rc  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*C6)))));
	if(ix < 0x3e99999a) 			/* if |x| < 0.3 */ 
	    rc = one - ((float)0.5*z - (z*rc - x*y));
	else {
	    if(ix > 0x3f480000) {		/* x > 0.78125 */
		qx = (float)0.28125;
	    } else {
	        SET_FLOAT_WORD(qx,ix-0x01000000);	/* x/4 */
	    }
	    hz = (float)0.5*z-qx;
	    a  = one-qx;
	    rc = a - (hz - (z*rc-x*y));
	}
    *sinptr = rs;
    *cosptr = rc;
}

void
__sincosf(float x, float *sptr, float *cptr)
{
    float y[2], z=0.0;
    int32_t n, ix;
    float rs, rc;

    /* high word of x */
    GET_FLOAT_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3f490fd8) {
        __kernel_sincosf(x, z, 0, sptr, cptr);
        return;
    }

    /* cos(Inf or NaN) is NaN */
    if (ix >= 0x7f800000) {
        float n = x - x;
        *sptr = *cptr = n;
        return;
    }
    /* argument reduction needed */
    n = __ieee754_rem_pio2f(x, y);
    __kernel_sincosf(y[0], y[1], 1, &rs, &rc);
    switch(n&3) {
    case 0:
        *sptr = rs; *cptr = rc;
        break;
    case 1:
        *sptr = rc; *cptr = -rs;
        break;
    case 2:
        *sptr = -rs; *cptr = -rc;
        break;
    default:
        *sptr = -rc; *cptr = rs;
        break;
    }
}

#if defined(__SHORT_DOUBLES_IMPL)
__strong_alias(__sincos,__sincosf);
#endif
