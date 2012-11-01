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
#define _POSIX_SOURCE
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

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
testtime(time_t t, const char *fmt, const char *expect, int local)
{
  struct tm tm;
  struct tm *tmp;
  char buf[128];
  time_t t2;

  if (local)
    tmp = localtime_r(&t, &tm);
  else
    tmp = gmtime_r(&t, &tm);

  assert(tmp == &tm);
  strftime(buf, sizeof(buf), fmt, tmp);
  checkstr(buf, expect);

  if (local) {
    t2 = mktime(&tm);
    assert(t2 == t);
  }
  printf("%s\n", buf);
}

#define testlocaltime(t, fmt, expect) testtime(t, fmt, expect, 1)
#define testgmttime(t, fmt, expect) testtime(t, fmt, expect, 0)

#define CLOCKS_PER_USEC (CLOCKS_PER_SEC/1000000)
/* function call overhead for usleep */
//#define OVERHEAD 1024
#define OVERHEAD 1100 /* bigger than it should be :-( */

/* test of sleep functions */
void
sleeptest(void)
{
  unsigned int now, delta;

  printf("testing sleep(1)... "); fflush(stdout);
  now = clock();
  sleep(1);
  delta = clock() - now;
  printf("delta = %u cycles ", delta); fflush(stdout);
  assert (delta >= CLOCKS_PER_SEC);
  assert (delta <= CLOCKS_PER_SEC + OVERHEAD);
  printf("ok\n");

  /* usleep() can really only provide accuracy to about 10 usec */
  printf("testing usleep(10000): "); fflush(stdout);
  now = clock();
  usleep(10000);
  delta = clock() - now;
  printf("delta = %u cycles ", delta); fflush(stdout);
  assert (delta >= 10000*CLOCKS_PER_USEC);
  assert (delta <=  OVERHEAD + 10000*CLOCKS_PER_USEC);
  printf("ok\n");

  printf("testing usleep(1000): "); fflush(stdout);
  now = clock();
  usleep(1000);
  delta = clock() - now;
  printf("delta = %u cycles ", delta); fflush(stdout);
  assert (delta >= 1000*CLOCKS_PER_USEC);
  assert (delta <=  OVERHEAD + 1000*CLOCKS_PER_USEC);
  printf("ok\n");

  printf("testing usleep(50): "); fflush(stdout);
  now = clock();
  usleep(50);
  delta = clock() - now;
  printf("delta = %u cycles ", delta); fflush(stdout);
  assert (delta >= 50*CLOCKS_PER_USEC);
  assert (delta <=  OVERHEAD + 50*CLOCKS_PER_USEC);
  printf("ok\n");


}

int
main()
{
  printf("sleep tests:\n");
  sleeptest();

  printf("time tests:\n");

  putenv("TZ=EST5EDT");
  printf("TZ=[%s]\n", getenv("TZ"));
  testgmttime(0, "%Y-%m-%d %H:%M:%S %a", "1970-01-01 00:00:00 Thu");
  testgmttime(63071999, "%F %r %a", "1971-12-31 11:59:59 PM Fri");
  testgmttime(63072000, "%F %T %a", "1972-01-01 00:00:00 Sat");
  testgmttime(951825660, "%F %T %a", "2000-02-29 12:01:00 Tue");
  testgmttime(951912060, "%F %T %a", "2000-03-01 12:01:00 Wed");
  testgmttime(1293840000, "%F %H:%M:%S %A", "2011-01-01 00:00:00 Saturday");
  testgmttime(1330559999, "%x %X", "02/29/12 23:59:59");
  testgmttime(1230465600, "%G-W%V-%u", "2008-W52-7");
  testgmttime(1230552000, "%G-W%V-%u", "2009-W01-1");
  testgmttime(1262174400, "%g-W%V-%u", "09-W53-3");
  testgmttime(1262433600, "%g-W%V-%u", "09-W53-6");
  testgmttime(1262692800, "%g-W%V-%u", "10-W01-2");
  testgmttime(1325419200, "%G-W%V-%u", "2011-W52-7");
  testgmttime(1325505600, "%G-W%V-%u", "2012-W01-1");
  testgmttime(4107585660, "%F %T %A", "2100-03-01 12:01:00 Monday");
  testgmttime(4260211200, "%F %T %A", "2105-01-01 00:00:00 Thursday");

  testlocaltime(1330559999, "%x %X %z", "02/29/12 18:59:59 -0500");
  testlocaltime(1330621200, "%j %B %r %Z", "061 March 12:00:00 PM EST");

  printf("ok\n");
  return 0;
}
