/**
 * @file include/complex.h
 * @brief Provides complex math API declarations.
 *
 * @details
 * Complex numbers are numbers of the form z = a+b*i, where a and b are
 * real numbers and i = sqrt(-1), so that i*i = -1.
 *
 * There are other ways to represent that number. The pair (a,b) of real
 * numbers may be viewed as a point in the plane, given by X- and
 * Y-coordinates.
 *
 * This same point may also be described by giving the pair of real numbers
 * (r,phi), where r is the distance to the origin O, and phi the angle
 * between the X-axis and the line Oz.
 * That is: z = r*exp(i*phi) = r*(cos(phi)+i*sin(phi)).
 *
 * @details
 * The basic operations are defined on z = a+b*i and w = c+d*i as:
 * @li addition: z+w = (a+c) + (b+d)*i
 * @li multiplication: z*w = (a*c - b*d) + (a*d + b*c)*i
 * @li division: z/w = ((a*c + b*d)/(c*c + d*d)) + ((b*c - a*d)/(c*c + d*d))*i
 *
 * Nearly all math function have a complex counterpart but there are some
 * complex-only functions.
 *
 * Your C-compiler can work with complex numbers if it supports the C99
 * standard. Link with -lm. The imaginary unit is represented by I.
 * 
 * Most detailed descriptions of these functions come from BSD man pages.
 * See file lib/LIB_LICENSE.txt for license info.
 */

#ifndef _COMPLEX_H
#define _COMPLEX_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__GNUC__)
#define _Complex_I (__extension__ 1.0iF)
#else
#define _Complex_I _Complex_I is not supported on this compiler
#endif

#define complex _Complex
#define I _Complex_I

#ifndef __ldouble_t
#if defined(__SHORT_DOUBLES_IMPL)
#define __ldouble_t double
#else
#define __ldouble_t long double
#endif
#endif

/**
 * @brief Complex inverse cosine function.
 *
 * cacos(z) computes the inverse cosine of the complex floating-point number
 * z, with branch cuts outside the interval [-1,1] along the real axis.
 *
 * cacos() returns values in a strip of the complex plane with unbounded
 * imaginary part, and real part in the interval [0, Pi].
 *
 * For all complex floating point numbers z, cacos(conj(z)) =
 * conj(cacos(z)).
 *
 * The conjugate symmetry of cacos() is used to abbreviate
 * the specification of special values.
 *
 * @li cacos(+-0 + 0i) returns Pi/2 - 0i.
 * @li cacos(+-0 + NaN i) returns Pi/2 + NaN i.
 * @li cacos(x + inf i) returns Pi/2 - inf i, for finite x.
 * @li cacos(x + NaN i) returns NaN + NaN i, for finite nonzero x.
 * @li cacos(-inf + yi) returns Pi - inf i, for finite positive-signed y.
 * @li cacos(inf + yi) returns 0 - inf i, for finite positive-signed y.
 * @li cacos(-inf + inf i) returns 3Pi/4 - inf i.
 * @li cacos(inf + inf i) returns Pi/4 - inf i.
 * @li cacos(+-inf + NaN i) returns NaN + inf i.
 * @li cacos(NaN + yi) returns NaN + NaN i, for finite y.
 * @li cacos(NaN + inf i) returns NaN - inf i.
 * @li cacos(NaN + NaN i) returns NaN + NaN i.
 *
 * @param z Number to take inverse cosine from.
 * @returns Inverse cosine of z.
 */
double complex cacos(double complex z);
/** See cacos() */
float complex cacosf(float complex z);
/** See cacos() */
__ldouble_t complex cacosl(__ldouble_t complex z);

/**
 * @brief Complex inverse sine function.
 *
 * casin(z) computes the inverse sine of the complex floating-point number
 * z, with branch cuts outside the interval [-1, 1] on the real axis.
 *
 * Function returns values in a strip of the complex plane with
 * unbounded imaginary part, and real part in the interval [-Pi/2, Pi/2].
 *
 *
 * casin are defined in terms of the complex inverse hyperbolic
 * functions as follows:
 *
 * @li casin(z) = -i * casinh(i*z),
 *
 * @param z Number for calculating the inverse sine.
 * @returns Inverse sine of z.
 */
double complex casin(double complex z);
/** See casin() */
float complex casinf(float complex z);
/** See casin() */
__ldouble_t complex casinl(__ldouble_t complex z);

/**
 * @brief Complex inverse tangent function
 *
 * ctan(z) computes the inverse tangent of the complex floating-point number
 * z, with branch cuts outside the interval [-i, i] on the imaginary axis.
 *
 * Function returns values in a strip of the complex plane with
 * unbounded imaginary part, and real part in the interval [-Pi/2, Pi/2].
 *
 *
 * catan is defined in terms of the complex inverse hyperbolic
 * function as follows:
 *
 * @li catan(z) = -i * catanh(i*z).
 *
 * @param z Number for calculating the inverse tangent.
 * @returns Inverse tan of z.
 */
double complex catan(double complex z);
/** See catan() */
float complex catanf(float complex z);
/** See catan() */
__ldouble_t complex catanl(__ldouble_t complex z);

/**
 * @brief Complex cosine function
 *
 * Computes the cosine of the complex floating-point number z.
 * @li ccos(z) = ccosh(i*z)
 * 
 * @param z Number for calculating cosine.
 * @returns complex cosine of z.
 */
double complex ccos(double complex z);
/** See ccos() */
float complex ccosf(float complex z);
/** See ccos() */
__ldouble_t complex ccosl(__ldouble_t complex z);

/**
 * @brief Complex sine function
 *
 * Computes the sine of the complex floating-point number z.
 * @li csin(z) = -i * csinh(i*z)
 * 
 * @param z Number for calculating sine.
 * @returns complex sine of z.
 */
double complex csin(double complex z);
/** See csin() */
float complex csinf(float complex z);
/** See csin() */
__ldouble_t complex csinl(__ldouble_t complex z);

/**
 * @brief Complex tangent function
 *
 * Computes the tangent of the complex floating-point number z.
 * @li ctan(z) = -i * ctanh(i*z)
 * 
 * @param z Number for calculating tangent.
 * @returns complex tangent of z.
 */
double complex ctan(double complex z);
/** See ctan() */
float complex ctanf(float complex z);
/** See ctan() */
__ldouble_t complex ctanl(__ldouble_t complex z);

/**
 * @brief complex inverse hyperbolic cosine function.
 *
 * cacosh(z) computes the inverse hyperbolic cosine of the complex
 * floating-point number z, with a branch cut on the interval
 * [-inf, 1] along the real axis.
 *
 * cacosh() returns values in a half-strip of the complex plane with
 * positive real part and imaginary part in the interval [-Pi, Pi].
 *
 * For all complex floating point numbers z,
 * cacosh(conj(z)) = * conj(cacosh(z)).
 *
 * The conjugate symmetry of cacosh() is used to abbreviate the
 * specification of special values.
 *
 * @li cacosh(+-0 + 0i) returns 0 + Pi/2 i.
 * @li cacosh(x + inf i) returns inf + Pi/2 i, for finite x.
 * @li cacosh(x + NaN i) returns NaN + NaN i, for finite nonzero x.
 * @li cacosh(-inf + yi) returns inf + Pi i, for finite positive-signed y.
 * @li cacosh(inf + yi) returns inf + 0i, for finite positive-signed y.
 * @li cacosh(-inf + inf i) returns inf + 3Pi/4 i.
 * @li cacosh(inf + inf i) returns inf + Pi/4 i.
 * @li cacosh(+-inf + NaN i) returns inf + NaN i.
 * @li cacosh(NaN + yi) returns NaN + NaN i, for finite y.
 * @li cacosh(NaN + inf i) returns inf + NaN i.
 * @li cacosh(NaN + NaN i) returns NaN + NaN i.
 *
 * @param z Complex number to calculate cacosh.
 * @returns Complex inverse hyperbolic cosine of z.
 */
double complex cacosh(double complex z);
/** See cacosh() */
float complex cacoshf(float complex z);
/** See cacosh() */
__ldouble_t complex cacoshl(__ldouble_t complex z);

/**
 * @brief Complex inverse hyperbolic sine function.
 *
 * casinh(z) computes the inverse hyperbolic sine of the complex
 * floating-point number z, with branch cuts outside the interval [-i, i]
 * along the imaginary axis.
 *
 * A branch cut is a curve (with ends possibly open, closed,
 * or half-open) in the complex plane across which an analytic
 * multivalued function is discontinuous.
 *
 * casinh() returns values in a strip of the complex plane with imaginary
 * part in the interval [-Pi/2, Pi/2].
 *
 * For all complex floating point numbers z,
 *
 * @li casinh(conj(z)) = conj(casinh(z)).
 * @li casinh(-z) = -casinh(z)
 *
 * @param z Complex number to calculate casinh.
 * @returns Complex inverse hyperbolic sine of z.
 */
double complex casinh(double complex z);
/** See casinh() */
float complex casinhf(float complex z);
/** See casinh() */
__ldouble_t complex casinhl(__ldouble_t complex z);

/**
 * @brief Complex inverse hyperbolic tangent function.
 *
 * catanh(z) computes the inverse hyperbolic tangent of the complex
 * floating-point number z, with branch cuts outside the interval [-1, 1]
 * along the real axis.
 * 
 * A branch cut is a curve (with ends possibly open, closed,
 * or half-open) in the complex plane across which an analytic
 * multivalued function is discontinuous.
 *
 * catanh() returns values in a strip of the complex plane with imaginary
 * part in the interval [-Pi/2, Pi/2].
 * 
 * For all complex floating point numbers z,
 * 
 * @li catanh(conj(z)) = conj(catanh(z))
 * @li catanh(-z) = -catanh(z)
 * 
 * @param z Complex number to calculate catanh.
 * @returns Complex inverse hyperbolic tangent of z.
 */
double complex catanh(double complex z);
/** See catanh() */
float complex catanhf(float complex z);
/** See catanh() */
__ldouble_t complex catanhl(__ldouble_t complex z);

/**
 * @brief Complex hyperbolic cosine function
 *
 * ccos(z) computes the hyperbolic cosine of the complex floating-point
 * number z.
 *
 * For all complex floating point numbers z,
 *
 * @li ccosh(conj(z)) = conj(ccosh(z))
 * @li ccosh(-z) = ccosh(z)
 *
 * The symmetries of ccosh() are used to abbreviate the specification of
 * special values.
 *
 * @li ccosh(0 + 0i) returns 1 + 0i.
 * @li ccosh(0 + inf i) returns NaN + 0i, and raises the invalid flag.
 * @li ccosh(0 + NaN i) returns NaN + 0i.
 * @li ccosh(x + inf i) returns NaN + NaN i, and raises the invalid flag, for finite nonzero x.
 * @li ccosh(x + NaN i) returns NaN + NaN i, for finite nonzero x.
 * @li ccosh(inf + 0i) returns inf + 0i.
 * @li ccosh(inf + yi) returns inf * cis(y), for finite positive y, where cis(y) = cos(y) + i*sin(y).
 * @li ccosh(inf + inf i) returns inf + NaN i, and raises the invalid flag.
 * @li ccosh(inf + NaN i) returns inf + NaN i.
 * @li ccosh(NaN + 0i) returns NaN + 0i.
 * @li ccosh(NaN + yi) returns NaN + NaN i, for nonzero numbers y.
 * @li ccosh(NaN + NaN i) returns NaN + NaN i.
 *
 * @param z Complex number to calculate ccosh.
 * @returns Complex hyperbolic cosine of z.
 */
double complex ccosh(double complex z);
/** See ccosh() */
float complex ccoshf(float complex z);
/** See ccosh() */
__ldouble_t complex ccoshl(__ldouble_t complex z);

/**
 * @brief Complex hyperbolic sine function
 *
 * csin(z) computes the hyperbolic sine of the complex floating-point number z.
 *
 * For all complex floating point numbers z,
 *
 * @li csinh(conj(z)) = conj(csinh(z))
 * @li csinh(-z) = -csinh(z)
 *
 * The symmetries of csinh() are used to abbreviate the specification of
 * special values.
 *
 * @li csinh(0 + 0i) returns 0 + 0i.
 * @li csinh(0 + inf i) returns 0 + NaN i, and raises the invalid flag.
 * @li csinh(0 + NaN i) returns 0 + NaN i.
 * @li csinh(x + inf i) returns NaN + NaN i, and raises the invalid flag, for finite nonzero x.
 * @li csinh(x + NaN i) returns NaN + NaN i, for finite nonzero x.
 * @li csinh(inf + 0i) returns inf + 0i.
 * @li csinh(inf + yi) returns inf * cis(y), for finite positive y, where cis(y) = cos(y) + i*sin(y).
 * @li csinh(inf + inf i) returns inf + NaN i, and raises the invalid flag.
 * @li csinh(inf + NaN i) returns inf + NaN i.
 * @li csinh(NaN + 0i) returns NaN + 0i.
 * @li csinh(NaN + yi) returns NaN + NaN i, for nonzero numbers y.
 * @li csinh(NaN + NaN i) returns NaN + NaN i.
 *
 * @param z Complex number to calculate csinh.
 * @returns Complex hyperbolic sine of z.
 */
double complex csinh(double complex z);
/** See csinh() */
float complex csinhf(float complex z);
/** See csinh() */
__ldouble_t complex csinhl(__ldouble_t complex z);

/**
 * @brief Complex hyperbolic tangent function
 *
 * ctanh(z) computes the hyperbolic tangent of the complex floating-point
 * number z.
 *
 * For all complex floating point numbers z,
 *
 * @li ctanh(conj(z)) = conj(ctanh(z)),
 * @li ctanh(-z) = -ctanh(z).
 *
 * The symmetries of ctanh() are used to abbreviate the specification of
 * special values.
 *
 * @li ctanh(0 + 0i) returns 0 + 0i.
 * @li ctanh(0 + inf i) returns NaN + NaN i, and raises the invalid flag.
 * @li ctanh(0 + NaN i) returns NaN + NaN i.
 * @li ctanh(x + inf i) returns NaN + NaN i, and raises the invalid flag, for finite nonzero x.
 * @li ctanh(x + NaN i) returns NaN + NaN i, for finite nonzero x.
 * @li ctanh(inf + 0i) returns 1 + 0i.
 * @li ctanh(inf + yi) returns 1 +- 0i, for finite positive y, with the sign chosen to match the sign of sin(2y).
 * @li ctanh(inf + inf i) returns 1 + 0i.
 * @li ctanh(inf + NaN i) returns 1 + 0i.
 * @li ctanh(NaN + 0i) returns NaN + 0i.
 * @li ctanh(NaN + yi) returns NaN + NaN i, for nonzero numbers y.
 * @li ctanh(NaN + NaN i) returns NaN + NaN i.
 *
 * @param z Complex number to calculate ctanh.
 * @returns Complex hyperbolic tangent of z.
 */
double complex ctanh(double complex z);
/** See ctanh() */
float complex ctanhf(float complex z);
/** See ctanh() */
__ldouble_t complex ctanhl(__ldouble_t complex z);


/*
 * exponential and logarithm functions
 */

/**
 * @brief Complex expotential function returns e raised to z power.
 *
 * This function calculates e (2.71828..., the base of natural logarithms)
 * raised to the power of z.
 *
 * In other words: e ** z such that ** means raise to power of.
 *
 * @param z The power to raise e.
 * @returns A double complex e to the power of z. f(z) = e ** z
 */
double complex cexp(double complex z);
/** See cexp() */
float complex cexpf(float complex z);
/** See cexp() */
__ldouble_t complex cexpl(__ldouble_t complex z);

/**
 * @brief Complex log function returns the natural logarithm of a complex number.
 *
 * The logarithm clog() is the inverse function of the exponential
 * cexp(). Thus, if y = clog(z), then z = cexp(y). The imaginary part
 * of y is chosen in the interval [-pi,pi].
 *
 * @param z Number to calculate the natuarl log.
 * @returns A double complex natural log of z. f(z) = ln(z)
 */
double complex clog(double complex z);
/** See clog() */
float complex clogf(float complex z);
/** See clog() */
__ldouble_t complex clogl(__ldouble_t complex z);

/*
 * power and absolute value
 */

/**
 * @brief Complex power function returns x to power of y.
 *
 * The function calculates x raised to the power y
 * with a branch cut for x along the negative real axis.
 *
 * A branch cut is a curve (with ends possibly open, closed,
 * or half-open) in the complex plane across which an analytic
 * multivalued function is discontinuous.
 *
 * @param x Number to raise.
 * @param y Power to raise x.
 * @returns A double complex x to the power of y. f(x,y) = x ** y
 */
double complex cpow(double complex x, double complex y);
/** See cpow() */
float complex cpowf(float complex x, float complex y);
/** See cpow() */
__ldouble_t complex cpowl(__ldouble_t complex x, __ldouble_t complex y);

/**
 * @brief Complex absolute value function.
 *
 * The cabs() function returns the absolute value of the complex number z.
 * The result is a real number.
 *
 * @param z Complex number to take the absolute value.
 * @returns Real double absolute value of z. f(z) = |z|
 */
double cabs(double complex z);
/** See cabs() */
float  cabsf(float complex z);
/** See cabs() */
__ldouble_t cabsl(__ldouble_t complex z);

/**
 * @brief Complex square root function.
 *
 * Calculate the square root of a given complex number, with nonnegative
 * real part, and with a branch cut along the negative real axis. That
 * means that csqrt(-1+eps*I) will be close to I while csqrt(-1-eps*I)
 * will be close to -I, if eps is a small positive real number.
 *
 * @param z Complex number to take square root.
 * @returns Complex square root of z.
 */
double complex csqrt(double complex z);
/** See csqrt() */
float complex csqrtf(float complex z);
/** See csqrt() */
__ldouble_t complex csqrtl(__ldouble_t complex z);

/*
 * manipulation functions
 */

/**
 * @brief Get the phase angle of the complex number z.
 * 
 * A complex number can be described by two real coordinates.
 *
 * One may use rectangular coordinates and get f(x,y) = x + I * y,
 * such that x = creal(z) and y = cimag(z).
 *
 * One may also use polar coordinates and get f(r,a) = r * cexp(I * a),
 * such that r = cabs(z) or the "radius" of z, and a = carg(z) or
 * the phase angle of z.
 *
 * @param z Number where angle is derived.
 * @returns Real number representing the phase angle of the complex number z.
 */
double carg(double complex z);
/** See carg() */
float  cargf(float complex z);
/** See carg() */
__ldouble_t cargl(__ldouble_t complex z);

/**
 * @brief Get the imaginary part of a complex number.
 *
 * This function returns the imaginary part of the complex number z.
 * Such that f(z) = creal(c) + I * cimag(z)
 *
 * @param z An complex number.
 * @returns A double representing the imaginary part of z.
 */
double cimag(double complex z);
/** See cimag() */
float  cimagf(float complex z);
/** See cimag() */
__ldouble_t cimagl(__ldouble_t complex z);

/**
 * @brief Get the real part of a complex number.
 *
 * This function returns the real part of the complex number z.
 * Such that f(z) = creal(c) + I * cimag(z)
 *
 * @param z A complex number.
 * @returns A double representing the real part of z.
 */
double creal(double complex z);
/** See creal() */
float  crealf(float complex z);
/** See creal() */
__ldouble_t creall(__ldouble_t complex z);

/**
 * @brief Calculate the complex conjugate of a complex number.
 *
 * The conj() function returns the complex conjugate value of z. That is
 * the value obtained by changing the sign of the imaginary part.
 * 
 * @param z A complex number
 * @returns The complex conjugate of the number.
 */
double complex conj(double complex z);
/** See conj() */
float complex conjf(float complex z);
/** See conj() */
__ldouble_t complex conjl(__ldouble_t complex z);

/**
 * @brief Project a point onto a a Riemann Sphere.
 *
 * This function projects a point in the plane onto the surface of a
 * Riemann Sphere, the one-point compactification of the complex plane.
 * Each finite point z projects to z itself.
 *
 * Every complex infinite value is projected to a single infinite value,
 * namely to positive infinity on the real axis.
 *
 * @param z A complex number.
 * @returns A single point on the Riemann sphere.
 */
double complex cproj(double complex z);
/** See cproj() */
float complex cprojf(float complex z);
/** See cproj() */
__ldouble_t complex cprojl(__ldouble_t complex z);

#if defined(__cplusplus)
}
#endif

#endif /* _COMPLEX_H */
