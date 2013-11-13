/*
 * Written by Eric R. Smith (ersmith@totalspectrum.ca)
 * and placed in the public domain
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <compiler.h>
#include <propeller.h>

/*
 * mktemp: replace trailing 6 XXXXXX with a unique value
 */

static HUBDATA int tmpnum;            /* incremented each time mktemp is called */
static HUBDATA _atomic_t mktemp_lock; /* to make this thread safe */

/* we use the name "_mktemp", which is in the ANSI reserved namespace,
 * so that the function can be called from standard C libraries;
 * a weak alias to "mktemp" is provided so that users can call it
 * directly if they want
 */
char *_mktemp(char *template)
{
  int numxs = 0;
  char *s;
  int xx, digit;
  int i;
  /* first, verify that the last 6 characters are XXXXXX */
  for (s = template; s && *s; s++) {
    if (*s == 'X')
      numxs++;
    else
      numxs = 0;
  }
  if (numxs < 6) {
    errno = EINVAL;
    *template = 0;
    return template;
  }
  /* right now "s" points at the trailing 0
     put it back 6 spaces to make room */
  s -= 6;

  /* atomically increment the temporary number */
  __lock(&mktemp_lock);
  xx = tmpnum++;
  __unlock(&mktemp_lock);

  /* create the template */
  for (i = 0; i < 6; i++) {
    digit = (xx & 0xF) + 'A';
    s[i] = digit;
    xx >>= 4;
  }
  return template;
}

__weak_alias(mktemp, _mktemp);
