/**
 * @file include/fenv.h
 *
 * @brief Provides floating point exception declarations.
 *
 * The descriptions here are only provided for completeness as the
 * functions are not intended to be called by normal user programs.
 *
 * The header <fenv.h> declares two types and several macros and functions
 * to provide access to the floating-point environment. The floating-point
 * environment refers collectively to any floating-point status flags
 * and control modes supported by the implementation.160) A floating-point
 * status flag is a system variable whose value is set as a side effect
 * of floating-point arithmetic to provide auxiliary information. A
 * floatingpoint control mode is a system variable whose value may be set
 * by the user to affect the subsequent behavior of floating-point arithmetic.
 *
 */
#ifndef _FENV_H
#define _FENV_H

#include <sys/thread.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Represents the floating-point exception flags collectively, including
 * any status the implementation associates with the flags.
 */
typedef unsigned char fexcept_t;

/**
 * Represents the entire floating-point environment.
 */
typedef _fenv_t fenv_t;

extern const fenv_t __dflt_fenv;

/**
 * Represents the default floating-point environment — the one installed
 * at program startup — and has type pointer to const-qualified fenv_t.
 * It can be used as an argument to <fenv.h> functions that manage the
 * floating-point environment.
 */
#define FE_DFL_ENV (&__dflt_fenv)

/** no floating point exceptions are defined */
#define FE_ALL_EXCEPT 0

/** only round-to-nearest mode supported */
#define FE_TONEAREST 0

/**
 * Clears the supported exceptions requested by its argument.
 */
int feclearexcept(int excepts);

/**
 * The fegetexceptflag function stores an implementation-defined
 * representation of the exception flags indicated by the argument
 * excepts in the object pointed to by the argument flagp.
 */
int fegetexceptflag(fexcept_t *flagp, int excepts);

/**
 * The feraiseexcept function raises the supported exceptions
 * represented by its argument. The order in which these exceptions
 * are raised is unspecified, except as stated in F.7.6. Whether the
 * feraiseexcept function additionally raises the inexact exception
 * whenever it raises the overflow or underflow exception is
 * implementation defined.
 */
int feraiseexcept(int excepts);

/**
 * The fesetexceptflag function sets the complete status for those
 * exception flags indicated by the argument excepts, according to the
 * representation in the object pointed to by flagp. The value of
 * *flagp shall have been set by a previous call to fegetexceptflag
 * whose second argument represented at least those exceptions
 * represented by the argument excepts. This function does not raise
 * exceptions, but only sets the state of the flags.
 */
int fesetexceptflag(const fexcept_t *flagp, int excepts);

/**
 * The fetestexcept function determines which of a specified subset
 * of the exception flags are currently set. The excepts argument
 * specifies the exception flags to be queried.
 */
int fetestexcept(int excepts);

/**
 * The fegetround function gets the current rounding direction.
 */
int fegetround(void);

/**
 * The fesetround function establishes the rounding direction
 * represented by its argument round. If the argument is not equal to
 * the value of a rounding direction macro, the rounding direction is
 * not changed.
 */
int fesetround(int round);

/**
 * Stores the current floating-point environment in the object pointed
 * to by envp.
 */
int fegetenv(fenv_t *envp);

/**
 * The feholdexcept function saves the current floating-point
 * environment in the object pointed to by envp, clears the exception
 * flags, and then installs a non-stop (continue on exceptions) mode,
 * if available, for all exceptions.
 */
int feholdexcept(fenv_t *envp);

/**
 * The fesetenv function establishes the floating-point environment
 * represented by the object pointed to by envp. The argument envp
 * shall point to an object set by a call to fegetenv or feholdexcept,
 * or equal the macro FE_DFL_ENV or an implementation-defined
 * environment macro. Note that fesetenv merely installs the state of
 * the exception flags represented through its argument, and does not
 * raise these exceptions.
 */
int fesetenv(const fenv_t *envp);

/**
 * The feupdateenv function saves the currently raised exceptions in
 * its automatic storage, installs the floating-point environment
 * represented by the object pointed to by envp, and then raises the
 * saved exceptions. The argument envp shall point to an object set
 * by a call to feholdexcept or fegetenv, or equal the macro FE_DFL_ENV
 * or an implementation-defined environment macro.
 */
int feupdateenv(const fenv_t *envp);

#if defined(__cplusplus)
}
#endif

#endif
