#include <fenv.h>

int
feclearexcept(int excepts)
{
  if (excepts == 0)
    return 0;
  return -1;
}

int
fegetexceptflag(fexcept_t *flagp, int excepts)
{
  if (excepts != 0)
    return -1;
  *flagp = 0;
  return 0;
}

int
feraiseexcept(int excepts)
{
  if (excepts == 0)
    return 0;
  return -1;
}

int
fesetexceptflag(const fexcept_t *flagp, int excepts)
{
  if (excepts == 0)
    return 0;
  return -1;
}

int
fetestexcept(int excepts)
{
  return 0;
}

int
fegetround(void)
{
  return FE_TONEAREST;
}

int
fesetround(int round)
{
  if (round == FE_TONEAREST)
    return 0;
  return -1;
}

int
fegetenv(fenv_t *envp)
{
  envp->exceptions = 0;
  envp->roundingmode = FE_TONEAREST;
  return 0;
}

int
feholdexcept(fenv_t *envp)
{
  /* no stop on exception is the default */
  return fegetenv(envp);
}

int
fesetenv(const fenv_t *envp)
{
  if (envp->exceptions == 0 && envp->roundingmode == FE_TONEAREST)
    return 0;
  return -1;
}

int
feupdateenv(const fenv_t *envp)
{
  return fesetenv(envp);
}
