
/* @(#)s_fabs.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

/*
 * fabs(x) returns the absolute value of x.
 */

#include "math.h"
#include "math_private.h"

double fabs(double x)
{
  u_int32_t ix;
  GET_HIGH_WORD(ix, x);
  SET_HIGH_WORD(x, ix & 0x7fffffff);
  return x;
}
