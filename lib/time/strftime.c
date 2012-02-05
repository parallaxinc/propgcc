/* from the MiNT library */
/*
 * strftime: print formatted information about a given time
 * Written by Eric R. Smith and placed in the public domain.
 *
 * With further modifications by Michal Jaegermann.
 * Formats in SYSV_EXT from PD strftime source by Arnold Robins.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

#define BIG_LEN 80

static int weeknumber (const struct tm *timeptr, int firstweekday);

#if 1
/* probably should be in a separate locale file, but put them here for now */
static char *_LC_Mth_name[] =
{ "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

static char *_LC_Day_name[] =
{
"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

#else
extern char **_LC_Mth_name;     /* in setlocale.c */
extern char **_LC_Day_name;
#endif

/*
 * FIXME: I'm not sure what the formats for 'c', 'x', and 'X' should be
 * for various locales, or even for the "C" locale. I've assumed that
 * 'c' gives the same as asctime(), 'x' is e.g. Jan 01 1970, and
 * 'X' is e.g. 12:20:37.
 *
 * I would venture that for "C" locale these formats are fine - mj.
 */

size_t
strftime(char *str, size_t maxsize, const char *fmt, const struct tm *ts)
{
        long	num = 0;
        long    len;
	int n;
        char    q;
        char    buf[BIG_LEN], *putstr, *s;

        for(;;) {
                if (num >= maxsize) return 0;
                if ((q = *fmt++) == 0)  break;
                if (q != '%') {
                        *str++ = q;
                        num++;
                        continue;
                }
                if ((q = *fmt++) == 0) break;       /* get format command */

/* assume that sprintf will be used, with a variable length */
/* this is the most common case, so it saves us some coding to do it here */
                putstr = buf;

                switch(q) {
                  case 'A':
                  case 'a':
		  /*
		   * cases 'a' and 'b' are still wrong -
		   * in some locale short names do not have to be
		   * 3 characters wide - still works in North America.
		   */
	                if (ts->tm_wday < 0 || ts->tm_wday > 6)
			        putstr = "?";
			else
			        if ( 'A' == q)
				        putstr = _LC_Day_name[ts->tm_wday];
				else
				        sprintf(buf, "%-.3s",
						_LC_Day_name[ts->tm_wday]);
                        break;
                  case 'B':
                  case 'b':
#ifdef SYSV_EXT
                  case 'h':	/* same as 'b' */
#endif
	                if (ts->tm_mon < 0 || ts->tm_mon > 11)
			        putstr = "?";
			else
			        if ( 'B' == q)
				        putstr = _LC_Mth_name[ts->tm_mon];
				else
				        sprintf(buf, "%-.3s",
						_LC_Mth_name[ts->tm_mon]);
                        break;
                  case 'c':
			/* this format should be set in setlocale.c */
                        strftime(buf, sizeof buf, "%a %b %d %X %Y", ts);
                        break;
		  case 'd':
                        sprintf(buf, "%02d", ts->tm_mday);
                        break;
		  case 'F':
		        sprintf(buf, "%04d-%02d-%02d", 1900+ts->tm_year, ts->tm_mon+1, ts->tm_mday);
			break;
                  case 'H':
                        sprintf(buf, "%02d", ts->tm_hour);
                        break;
                  case 'I':
                        n = ts->tm_hour;
                        if (n == 0) n = 12;
			else if (n > 12) n -= 12;
                        sprintf(buf, "%02d", n);
                        break;
                  case 'j':
                        sprintf(buf, "%03d", ts->tm_yday + 1);
                        break;
                  case 'm':
                        sprintf(buf, "%02d", ts->tm_mon + 1);
                        break;
                  case 'M':
                        sprintf(buf, "%02d", ts->tm_min);
                        break;
                  case 'p':
			/*
			 * this is wrong - strings "AM', "PM" are
			 * locale dependent
			 */
                        putstr = (ts->tm_hour < 12) ? "AM" : "PM";
                        break;
                  case 'S':
                        sprintf(buf, "%02d", ts->tm_sec);
                        break;
                  case 'U':  /* week of year - starting Sunday */
			sprintf(buf, "%02d", weeknumber(ts, 0));
                        break;
                  case 'W': /* week of year - starting Monday */
			sprintf(buf, "%02d", weeknumber(ts, 1));
                        break;
                  case 'w':
                        sprintf(buf, "%d", ts->tm_wday);
                        break;
                  case 'x':
		        /* once again - format may be locale dependent */ 
                        strftime(buf, sizeof buf, "%b %d %Y", ts);
                        break;
                  case 'X':
			/* same applies to this format as well */
                        sprintf(buf, "%02d:%02d:%02d", ts->tm_hour,
                                ts->tm_min, ts->tm_sec);
                        break;
                  case 'y':
                        sprintf(buf, "%02d", ts->tm_year % 100);
                        break;
		  case 'Y':
                        sprintf(buf, "%d", ts->tm_year + 1900);
                        break;
                  case 'Z':
                        if (NULL != (s = getenv("TZ"))) {
				strcpy(buf, s);
				s = buf;
				if (ts->tm_isdst) {
				    while (*s && isalpha(*s)) s++;
					    while (*s && (isdigit(*s)||
							  *s == '+'  ||
							  *s == '-'  ||
							  *s == ':')) {
						    s++;
					    };
					    if (*s)
					        putstr = s;
				}
				s = putstr;
				while (*s && isalpha(*s))
				        s++;
				*s = 0;
			}
			else
			        buf[0] = '\0'; /* empty string */
                        break;
                   case '%':
                        putstr = "%";
                        break;
#ifdef SYSV_EXT
		   case 'n':	/* same as \n */
			putstr = "\n";
			break;
		   case 't':	/* same as \t */
			putstr = "\t";
			break;
		   case 'D':	/* date as %m/%d/%y */
			strftime(buf, sizeof buf, "%m/%d/%y", ts);
			break;
		   case 'e':	/* day of month, blank padded */
			sprintf(buf, "%2d", ts->tm_mday);
			break;
		   case 'r':	/* time as %I:%M:%S %p */
			strftime(buf, sizeof buf, "%I:%M:%S %p", ts);
			break;
		   case 'R':	/* time as %H:%M */
			strftime(buf, sizeof buf, "%H:%M", ts);
			break;
		   case 'T':	/* time as %H:%M:%S */
			strftime(buf, sizeof buf, "%H:%M:%S", ts);
			break;
#endif
		   default:
			buf[0] = q;
			buf[1] = 0;
			break;
                }

                if (num + (len = strlen(putstr)) >= maxsize)
                        return 0;

                num += len;
                while (--len >= 0)
                        *str++ = *putstr++;
        }
        *str = 0;
        return (size_t) num;
}

/* 
 * What follows grabbed, with a small change, from PD source of strftime
 * by Arnold Robins - arnold@audiofax.com
 */
/* With thanks and tip of the hatlo to ado@elsie.nci.nih.gov */

static int
weeknumber(const struct tm *timeptr, int firstweekday)
/*
 * firstweekday is 0 if starting in Sunday, non-zero if in Monday
 */
{
    return (timeptr->tm_yday - timeptr->tm_wday +
	    (firstweekday ? (timeptr->tm_wday ? 8 : 1) : 7)) / 7;
}
