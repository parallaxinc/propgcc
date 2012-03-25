#ifndef _FENV_H
#define _FENV_H

#include <sys/thread.h>

#if defined(__cplusplus)
extern "C" {
#endif

  typedef unsigned char fexcept_t;
  typedef _fenv_t fenv_t;

  extern const fenv_t __dflt_fenv;

#define FE_DFL_ENV (&__dflt_fenv)

/* no floating point exceptions are defined */
#define FE_ALL_EXCEPT 0

/* only round-to-nearest mode supported */
#define FE_TONEAREST 0

  int feclearexcept(int excepts);
  int fegetexceptflag(fexcept_t *flagp, int excepts);
  int feraiseexcept(int excepts);
  int fesetexceptflag(const fexcept_t *flagp, int excepts);
  int fetestexcept(int excepts);
  int fegetround(void);
  int fesetround(int round);

  int fegetenv(fenv_t *envp);
  int feholdexcept(fenv_t *envp);
  int fesetenv(const fenv_t *envp);
  int feupdateenv(const fenv_t *envp);

#if defined(__cplusplus)
}
#endif

#endif
