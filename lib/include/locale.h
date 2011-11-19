/*
 * from the MiNT library (public domain)
 */
/*
 * locale.h (ansi draft sec 4.4)
 *      not implemented
 */

#ifndef _LOCALE_H
#define _LOCALE_H

#ifdef __cplusplus
extern "C" {
#endif

#define LC_ALL          0x001F
#define LC_COLLATE      0x0001
#define LC_CTYPE        0x0002
#define LC_MONETARY     0x0004
#define LC_NUMERIC      0x0008
#define LC_TIME         0x0010

#ifndef NULL
#include <sys/null.h>
#endif

struct lconv {
        char    *decimal_point;
        char    *thousands_sep;
        char    grouping;
        char    *int_curr_symbol;
        char    *currency_symbol;
        char    *mon_decimal_point;
        char    *mon_thousands_sep;
        char    mon_grouping;
        char    *positive_sign;
        char    *negative_sign;
        char    int_frac_digits;
        char    frac_digits;
        char    p_cs_precedes;
        char    p_sep_by_space;
        char    n_cs_precedes;
        char    n_sep_by_space;
        char    p_sign_posn;
        char    n_sign_posn;
};

char *		setlocale(int category, const char *locale);
/* default is supposed to be setlocale(LC_ALL, "C") */

struct lconv *	localeconv(void);

#ifdef __cplusplus
}
#endif

#endif /* _LOCALE_H */

