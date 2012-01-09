/*
 * From the MiNT library
 *
 * Routines for handling the local environment.
 * WARNING: This probably isn't in accord with the ANSI standard yet.
 *
 * Written by Eric R. Smith and placed in the public domain.
 *
 */

#include <stddef.h>
#include <string.h>
#include <locale.h>
#include <limits.h>

/* locale structure */
/* empty strings, or values of CHAR_MAX, indicate the item is not available
 * in this locale
 */

static struct lconv C_locale = {
        ".",    /* decimal point for ordinary numbers */
        "",     /* thousands separator */
        "",     /* how digits in ordinary numbers are grouped */
        "",     /* decimal point for money */
        "",     /* thousands separator for money */
        "",     /* how digits in a monetary value are grouped */
        "",     /* symbol for positive amount of money */
        "",     /* symbol for negative amount of money */
        "",     /* currency symbol for ordinary use */

        CHAR_MAX,      /* local: number of places after '.' for money */
        CHAR_MAX,      /* currency symbol 1 precedes 0 succeeds positive value */
        CHAR_MAX,      /* currency symbol 1 precedes 0 succeeds neg. value */
        CHAR_MAX,      /* 1=space 0=no space between currency symbol and pos. value */
        CHAR_MAX,      /* 1=space 0=no space between currency symbol and neg. value */
        CHAR_MAX,      /* position of sign in postive money values (???) */
        CHAR_MAX,      /* position of sign in negative money values (???) */

	"",         /* currency symbol for international use */

        CHAR_MAX,      /* international: number of places after '.' for money */
        CHAR_MAX,      /* currency symbol 1 precedes 0 succeeds positive value */
        CHAR_MAX,      /* currency symbol 1 precedes 0 succeeds neg. value */
        CHAR_MAX,      /* 1=space 0=no space between currency symbol and pos. value */
        CHAR_MAX,      /* 1=space 0=no space between currency symbol and neg. value */
        CHAR_MAX,      /* position of sign in postive money values (???) */
        CHAR_MAX,       /* position of sign in negative money values (???) */

};

/* current locale info */
static struct lconv _LC_Curlocale;

static int localeset = 0;

/* localeconv: return current locale information */

struct lconv *localeconv(void)
{
        if (localeset == 0) {
                _LC_Curlocale = C_locale;
                localeset = 1;
        }
        return &_LC_Curlocale;
}

/* setlocale: set the locale.
 * FIXME: right now, only "C" is supported.
 */

char *setlocale(int category, const char *name)
{
        if (name && strcmp(name, "C"))
                return (char *)0;

        if (!localeset) {
                localeset = 1;
                _LC_Curlocale = C_locale;
        }

/* here's where we usually would change things */
        return "C";
}
