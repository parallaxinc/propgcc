/*
 * various test routines for time-related functions
 * Copyright (c) 2012 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define string__(x) #x
#define string_(x) string__(x)

static void checkstr(const char *s1, const char *expect)
{
  if (strcmp(s1, expect) != 0) {
    fprintf(stderr, "check failed: expected [%s] got [%s]\n", expect, s1);
    abort();
  }
}

void
testtime(time_t t, const char *fmt, const char *expect)
{
  struct tm tm;
  struct tm *tmp;
  char buf[128];
  time_t t2;

  tmp = gmtime_r(&t, &tm);
  assert(tmp == &tm);
  strftime(buf, sizeof(buf), fmt, tmp);
  checkstr(buf, expect);

  t2 = mktime(&tm);
  assert(t2 == t);
  printf("%s\n", buf);
}

int
main()
{
#if !defined(__propeller__)
  putenv("TZ=GMT");
#endif
  printf("time tests:\n");
  testtime(0, "%Y-%m-%d %H:%M:%S %a", "1970-01-01 00:00:00 Thu");
  testtime(63071999, "%F %H:%M:%S %a", "1971-12-31 23:59:59 Fri");
  testtime(63072000, "%F %H:%M:%S %a", "1972-01-01 00:00:00 Sat");
  testtime(1293840000, "%F %H:%M:%S %a", "2011-01-01 00:00:00 Sat");
  printf("ok\n");
  return 0;
}
