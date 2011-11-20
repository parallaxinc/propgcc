/*
 * From the MiNT library
 *
 * Routines for handling the local environment.
 * WARNING: This probably isn't in accord with the pANS standard yet.
 *
 * Written by Eric R. Smith and placed in the public domain.
 *
 */

#include <stddef.h>
#include <string.h>
#include <locale.h>

static struct lconv C_locale = {
        ".",    /* decimal point for ordinary numbers */
        ",",    /* thousands separator */
        3,      /* how digits in ordinary numbers are grouped */
        "$",    /* international currency symbol */
        "$",    /* currency symbol for ordinary use */
        ".",    /* decimal point for money */
        ",",    /* thousands separator for money */
        3,      /* how digits in a monetary value are grouped */
        "",     /* symbol for positive amount of money */
        "-",    /* symbol for negative amount of money */
        4,      /* International: number of places after '.' for money*/
        2,      /* local: number of places after '.' for money */
        1,      /* currency symbol 1 precedes 0 succeeds positive value */
        1,      /* 1=space 0=no space between currency symbol and pos. value */
        1,      /* currency symbol 1 precedes 0 succeeds neg. value */
        0,      /* 1=space 0=no space between currency symbol and neg. value */
        1,      /* position of sign in postive money values (???) */
        1       /* position of sign in negative money values (???) */
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
