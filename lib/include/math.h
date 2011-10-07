#ifndef _MATH_H
#define _MATH_H

/************************************************************************
 *									*
 *				N O T I C E				*
 *									*
 *			Copyright Abandoned, 1987, Fred Fish		*
 *									*
 *	This previously copyrighted work has been placed into the	*
 *	public domain by the author (Fred Fish) and may be freely used	*
 *	for any purpose, private or commercial.  I would appreciate	*
 *	it, as a courtesy, if this notice is left in all copies and	*
 *	derivative works.  Thank you, and enjoy...			*
 *									*
 *	The author makes no warranty of any kind with respect to this	*
 *	product and explicitly disclaims any implied warranties of	*
 *	merchantability or fitness for any particular purpose.		*
 *									*
 ************************************************************************
 */


/*
 *  FILE
 *
 *	math.h    include file for users of portable math library
 *
 *  SYNOPSIS
 *
 *	#include <math.h>
 *
 *  DESCRIPTION
 *
 *	This file should be included in any user compilation module
 *	which accesses routines from the Portable Math Library (PML).
 *
 */


#ifndef _COMPILER_H
#include <compiler.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define M_LN2                0.69314718055994530942
#define M_PI         3.14159265358979323846
#define M_SQRT2              1.41421356237309504880
#define M_E          2.7182818284590452354
#define M_LOG2E              1.4426950408889634074
#define M_LOG10E     0.43429448190325182765
#define M_LN10               2.30258509299404568402
#define M_PI_2               1.57079632679489661923
#define M_PI_4               0.78539816339744830962
#define M_1_PI               0.31830988618379067154
#define M_2_PI               0.63661977236758134308
#define M_2_SQRTPI   1.12837916709551257390
#define M_SQRT1_2    0.70710678118654752440

#ifdef __GNUC__
#define HUGE_VAL __builtin_huge_val()
#define HUGE_VALL __builtin_huge_vall();
#else
extern const double _infinitydf;	/* in normdf.cpp */
#define HUGE_VAL  (_infinitydf)
#define HUGE_VALL (_infinitydf)
#endif

#define HUGE HUGE_VAL

#ifdef __GNUC__
# ifndef __cplusplus
#  ifndef max
#   define max(x,y) ({typeof(x) _x=(x); typeof(y) _y=(y); if (_x>_y) _y=_x; _y;})
#   define min(x,y) ({typeof(x) _x=(x); typeof(y) _y=(y); if (_x<_y) _y=_x; _y;})
#  endif
# endif
#endif


 double sin	(double);
 double cos	(double);
 double tan	(double);
 double asin	(double);
 double	acos	(double);
 double atan	(double);
 double atan2	(double, double);
 double sinh	(double);
 double cosh	(double);
 double tanh	(double);
 double atanh	(double);
 double exp	(double);
 double log	(double);
 double log10	(double);
 double sqrt	(double);
 double hypot   (double, double);
 double pow	(double, double);
 double fabs	(double);
 double ceil	(double);
 double floor	(double);
 double rint	(double);
 double fmod	(double, double);

 double ldexp	(double, int);
 double frexp	(double, int *);
 double modf	(double, double *);

 double acosh   (double);
 double asinh   (double);

 double copysign(double, double);

#ifdef __cplusplus
}
#endif

#endif /* _MATH_H */
