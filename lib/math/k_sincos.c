/* @(#)k_sincos.c 5.1 93/09/24 */
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

/* Adapted from k_sin.c and k_cos.c by Eric Smith
 * ersmith@totalspectrum.ca
 */

#include "math.h"
#include "math_private.h"

static const double 
half =  5.00000000000000000000e-01, /* 0x3FE00000, 0x00000000 */
S1  = -1.66666666666666324348e-01, /* 0xBFC55555, 0x55555549 */
S2  =  8.33333333332248946124e-03, /* 0x3F811111, 0x1110F8A6 */
S3  = -1.98412698298579493134e-04, /* 0xBF2A01A0, 0x19C161D5 */
S4  =  2.75573137070700676789e-06, /* 0x3EC71DE3, 0x57B1FE7D */
S5  = -2.50507602534068634195e-08, /* 0xBE5AE5E6, 0x8A2B9CEB */
S6  =  1.58969099521155010221e-10, /* 0x3DE5D93A, 0x5ACFD57C */
one =  1.00000000000000000000e+00, /* 0x3FF00000, 0x00000000 */
C1  =  4.16666666666666019037e-02, /* 0x3FA55555, 0x5555554C */
C2  = -1.38888888888741095749e-03, /* 0xBF56C16C, 0x16C15177 */
C3  =  2.48015872894767294178e-05, /* 0x3EFA01A0, 0x19CB1590 */
C4  = -2.75573143513906633035e-07, /* 0xBE927E4F, 0x809C52AD */
C5  =  2.08757232129817482790e-09, /* 0x3E21EE9E, 0xBDB4B1C4 */
C6  = -1.13596475577881948265e-11; /* 0xBDA8FAE9, 0xBE8838D4 */


static void
__kernel_sincos(double x, double y, int iy, double *sptr, double *cptr)
{
	double z,rs, rc,v;
    double a, hz, qx;
	int32_t ix;
	GET_HIGH_WORD(ix,x);
	ix &= 0x7fffffff;			/* high word of x */
	if(ix<0x3e400000) {			/* |x| < 2**-27 */
        if((int)x==0) {
            *sptr = x;
            *cptr = one;
            return;
        }
    }
	z	=  x*x;
	v	=  z*x;
	rs	=  S2+z*(S3+z*(S4+z*(S5+z*S6)));
	rc  = z*(C1+z*(C2+z*(C3+z*(C4+z*(C5+z*C6)))));
	if(iy==0) rs = x+v*(S1+z*rs);
	else      rs = x-((z*(half*y-v*rs)-y)-v*S1);

	if(ix < 0x3FD33333) 			/* if |x| < 0.3 */ 
	    rc = one - (0.5*z - (z*rc - x*y));
	else {
	    if(ix > 0x3fe90000) {		/* x > 0.78125 */
		qx = 0.28125;
	    } else {
	        INSERT_WORDS(qx,ix-0x00200000,0);	/* x/4 */
	    }
	    hz = 0.5*z-qx;
	    a  = one-qx;
	    rc = a - (hz - (z*rc-x*y));
	}
    *sptr = rs;
    *cptr = rc;
}

void
__sincosl(double x, double *sptr, double *cptr)
{
    double y[2], z=0.0;
    int32_t n, ix;
    double rs, rc;

    /* high word of x */
    GET_HIGH_WORD(ix, x);

    /* |x| ~< pi/4 */
    ix &= 0x7fffffff;
    if (ix <= 0x3fe921fb) {
        __kernel_sincos(x, z, 0, sptr, cptr);
        return;
    }

    /* cos(Inf or NaN) is NaN */
    if (ix >= 0x7ff00000) {
        double n = x - x;
        *sptr = *cptr = n;
        return;
    }
    /* argument reduction needed */
    n = __ieee754_rem_pio2(x, y);
    __kernel_sincos(y[0], y[1], 1, &rs, &rc);
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

#if !defined(__SHORT_DOUBLES_IMPL)
__strong_alias(__sincos,__sincosl);
#endif
