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

  double complex cacos(double complex z);
  float complex cacosf(float complex z);
  __ldouble_t complex cacosl(__ldouble_t complex z);

  double complex casin(double complex z);
  float complex casinf(float complex z);
  __ldouble_t complex casinl(__ldouble_t complex z);

  double complex catan(double complex z);
  float complex catanf(float complex z);
  __ldouble_t complex catanl(__ldouble_t complex z);

  double complex ccos(double complex z);
  float complex ccosf(float complex z);
  __ldouble_t complex ccosl(__ldouble_t complex z);

  double complex csin(double complex z);
  float complex csinf(float complex z);
  __ldouble_t complex csinl(__ldouble_t complex z);

  double complex ctan(double complex z);
  float complex ctanf(float complex z);
  __ldouble_t complex ctanl(__ldouble_t complex z);

  double complex cacosh(double complex z);
  float complex cacoshf(float complex z);
  __ldouble_t complex cacoshl(__ldouble_t complex z);

  double complex casinh(double complex z);
  float complex casinhf(float complex z);
  __ldouble_t complex casinhl(__ldouble_t complex z);

  double complex catanh(double complex z);
  float complex catanhf(float complex z);
  __ldouble_t complex catanhl(__ldouble_t complex z);

  double complex ccosh(double complex z);
  float complex ccoshf(float complex z);
  __ldouble_t complex ccoshl(__ldouble_t complex z);

  double complex csinh(double complex z);
  float complex csinhf(float complex z);
  __ldouble_t complex csinhl(__ldouble_t complex z);

  double complex ctanh(double complex z);
  float complex ctanhf(float complex z);
  __ldouble_t complex ctanhl(__ldouble_t complex z);

  /* exponential and logarithm functions */
  double complex cexp(double complex z);
  float complex cexpf(float complex z);
  __ldouble_t complex cexpl(__ldouble_t complex z);

  double complex clog(double complex z);
  float complex clogf(float complex z);
  __ldouble_t complex clogl(__ldouble_t complex z);

  /* power and absolute value */
  double complex cpow(double complex x, double complex y);
  float complex cpowf(float complex x, float complex y);
  __ldouble_t complex cpowl(__ldouble_t complex x, __ldouble_t complex y);

  double cabs(double complex z);
  float  cabsf(float complex z);
  __ldouble_t cabsl(__ldouble_t complex z);

  double complex csqrt(double complex z);
  float complex csqrtf(float complex z);
  __ldouble_t complex csqrtl(__ldouble_t complex z);

  /* manipulation functions */
  double carg(double complex z);
  float  cargf(float complex z);
  __ldouble_t cargl(__ldouble_t complex z);

  double cimag(double complex z);
  float  cimagf(float complex z);
  __ldouble_t cimagl(__ldouble_t complex z);
  double creal(double complex z);
  float  crealf(float complex z);
  __ldouble_t creall(__ldouble_t complex z);

  double complex conj(double complex z);
  float complex conjf(float complex z);
  __ldouble_t complex conjl(__ldouble_t complex z);

  double complex cproj(double complex z);
  float complex cprojf(float complex z);
  __ldouble_t complex cprojl(__ldouble_t complex z);

#if defined(__cplusplus)
}
#endif

#endif /* _COMPLEX_H */
