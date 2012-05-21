/**
 * @file include/ctype.h
 *
 * @brief Provides character class check API macros.
 *
 * @details
 * These functions check whether c, which must have the value of an
 * unsigned char or EOF, falls into a certain character class according
 * to the current locale.
 *
 * @details
 * Conforms to C99, 4.3BSD. C89 specifies all of these functions except isascii() and
 * isblank(). isascii() is a BSD extension and is also an SVr4 extension.
 * isblank() conforms to POSIX.1-2001 and C99 7.4.1.3. POSIX.1-2008 marks
 * isascii() as obsolete, noting that it cannot be used portably in a
 * localized application.
 *
 */
/*
 * doxygen note.
 * \@file uses include/ctype.h to differentiate from include/sys/ctype.h
 * Without this, neither file will be documented.
 */


#ifndef _CTYPE_H
#define _CTYPE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <sys/ctype.h>

  extern int isalnum(int c);
  extern int isalpha(int c);
  extern int isblank(int c);
  extern int iscntrl(int c);
  extern int isdigit(int c);
  extern int isgraph(int c);
  extern int islower(int c);
  extern int isprint(int c);
  extern int ispunct(int c);
  extern int isspace(int c);
  extern int isupper(int c);
  extern int isxdigit(int c);

  extern int tolower(int c);
  extern int toupper(int c);


#define __isctype(c, x) (__ctype_get(c) & x)

/**
 * Checks for an alphanumeric class character.
 */
#define isalnum(c)     __isctype(c, _CTalnum)
/**
 * Checks for an alphabetic class character.
 *
 * The isalpha function tests for any character for which isupper or
 * islower is true, or any character that is one of a locale-specific
 * set of alphabetic characters for which none of iscntrl, isdigit,
 * ispunct, or isspace is true.156) In the "C" locale, isalpha returns
 * true only for the characters for which isupper or islower is true.
 */
#define isalpha(c)     __isctype(c, _CTalpha)
/**
 * Checks for a blank (space or tab) character.
 */
#define isblank(c)     __isctype(c, _CTb)
/**
 * Checks for a control character.
 */
#define iscntrl(c)     __isctype(c, _CTc)
/**
 * Checks for a digit (0 through 9) character.
 */
#define isdigit(c)     __isctype(c, _CTd)
/**
 * Checks for a any printable character except for space.
 */
#define isgraph(c)     (!__isctype(c, (_CTc|_CTs)) && (__ctype_get(c) != 0))
/**
 * Checks for a lower-case character.
 *
 * The islower function tests for any character that is a lowercase
 * letter or is one of a locale-specific set of characters for which
 * none of iscntrl, isdigit, ispunct, or isspace is true. In the * "C"
 * locale, islower returns true only for the characters defined as
 * lowercase letters.
 */
#define islower(c)     __isctype(c, _CTl)
/**
 * Checks for any printable character including space.
 */
#define isprint(c)     (!__isctype(c, (_CTc)) && (__ctype_get(c) != 0))
/**
 * Checks for any printable character that is not a space or alphanumeric character.
 *
 * The ispunct function tests for any printing character that is one of
 * a locale-specific set of punctuation characters for which neither
 * isspace nor isalnum is true.
 */
#define ispunct(c)     __isctype(c, _CTp)
/**
 * Checks for a white space character: space, \\f, \\n, \\r, \\t, \\v in "C" locales.
 */
#define isspace(c)     __isctype(c, _CTs)
/**
 * Checks for an upper-case character.
 *
 * The isupper function tests for any character that is an uppercase
 * letter or is one of a locale-specific set of characters for which
 * none of iscntrl, isdigit, ispunct, or isspace is true. In the "C"
 * locale, isupper returns true only for the characters defined as
 * uppercase letters.
 */
#define isupper(c)     __isctype(c, _CTu)
/**
 * Checks for a hexadecimal digit: 0 through 9, a through f, or A through F.
 */
#define isxdigit(c)    __isctype(c, _CTx)

/**
 * Converts an uppercase letter to a corresponding lowercase letter.
 *
 * If the argument is a character for which isupper is true and there
 * are one or more corresponding characters, as specified by the current
 * locale, for which islower is true, the tolower function returns one
 * of the corresponding characters (always the same one for any given
 * locale); otherwise, the argument is returned unchanged.
 *
 * @param c Letter to convert.
 * @returns Uppercase version of c according to locale.
 */
int tolower(int c);

/**
 * Converts a lowercase letter to a corresponding uppercase letter.
 *
 * If the argument is a character for which islower is true and there
 * are one or more corresponding characters, as specified by the current
 * locale, for which isupper is true, the toupper function returns one
 * of the corresponding characters (always the same one for any given
 * locale); otherwise, the argument is returned unchanged.
 *
 * @param c Letter to convert.
 * @returns Lowercase version of c according to locale.
 */
int toupper(int c);

#if defined(__cplusplus)
}
#endif

#endif
