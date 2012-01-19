/*
 * Implementation of wide character functions
 *
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
#include <stdio.h>
#include <assert.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define string__(x) #x
#define string_(x) string__(x)

static void docheck(unsigned long long a, unsigned long long expect)
{
  if (a != expect) {
    fprintf(stderr, "check failed: expected 0x%llx, got 0x%llx\n", expect, a);
    exit(1);
  }
}

#define check(x, expect) docheck((unsigned long long)x, (unsigned long long)expect)


void
testuns(const char *str, unsigned long expect, const wchar_t *lstr, unsigned long long lexpect, int base)
{
  unsigned long xl;
  unsigned long long xll;

  printf("testing [%s, %d] strtoul ", str, base); fflush(stdout);
  xl = strtoul(str, NULL, base);
  check(xl, expect);
  printf("strtoull "); fflush(stdout);
  xll = strtoull(str, NULL, base);
  check(xll, lexpect);

  printf("wcstoul "); fflush(stdout);
  xl = wcstoul(lstr, NULL, base);
  check(xl, expect);
  printf("wcstoull "); fflush(stdout);
  xll = wcstoull(lstr, NULL, base);
  check(xll, lexpect);

  if (base == 8 || base == 10 || base == 16 || base == 0) {
    printf("scanf "); fflush(stdout);
    switch (base) {
    case 8:
      sscanf(str, "%lo", &xl);
      sscanf(str, "%llo", &xll);
      break;
    case 10:
      sscanf(str, "%lu", &xl);
      sscanf(str, "%llu", &xll);
      break;
    case 16:
      sscanf(str, "%lx", &xl);
      sscanf(str, "%llx", &xll);
      break;
    default:
      sscanf(str, "%li", &xl);
      sscanf(str, "%lli", &xll);
      break;
    }
    /* NOTE: scanf does not have to handle overwrap */
    if (expect <= LONG_MAX)
      check(xl, expect);
    check(xll, lexpect);
  }
  printf("ok\n");

}

int
main()
{
  testuns("0", 0, L"0", 0LL, 10);
  testuns("0x10", 16, L"0x10", 16LL, 0);
  testuns("0x10", 0, L"0x10", 0LL, 10);
  testuns("ABC", 0xABC, L"ABC", 0xABCLL, 16);
  testuns("011", 3, L"011", 3LL, 2);
  testuns("41", 33, L"41", 33LL, 8); 
  testuns("99A", 99, L"99x", 99LL, 10);
  testuns("-99", 0xFFFFFF9D, L"-99", -99LL, 10);
  testuns("80000000", 0x80000000UL, L"80000000", 0x80000000LL, 16);
  testuns("4294967295", 0xFFFFFFFFUL, L"4294967295", 0xFFFFFFFFULL, 10);
  testuns("4294967296", 0xFFFFFFFFUL, L"4294967296", 0x100000000ULL, 10);
  testuns("-1", 0xFFFFFFFFUL, L"-1", 0xFFFFFFFFFFFFFFFFULL, 10);

  return 0;
}
