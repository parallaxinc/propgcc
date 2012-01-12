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

#define string__(x) #x
#define string_(x) string__(x)

static void docheck(int condition, const char *msg, unsigned long long x)
{
  if (!condition) {
    fprintf(stderr, "%s failed for value 0x%llx\n", msg, x);
    abort();
  }
}

#define check(cond, x) docheck(cond, string_(cond), x)


void
testuns(const char *str, const wchar_t *lstr, unsigned long long expect, int base)
{
  unsigned long xl;
  unsigned long long xll;

  printf("testing [%s, %d]\n", str, base);
  xl = strtoul(str, NULL, base);
  check((unsigned long long)xl == expect, expect);
  xl = wcstoul(lstr, NULL, base);
  check((unsigned long long)xl == expect, expect);

  xll = strtoul(str, NULL, base);
  check(xll == expect, expect);
  xll = wcstoul(lstr, NULL, base);
  check(xll == expect, expect);


}

int
main()
{
  testuns("0", L"0", 0LL, 10);
  testuns("0x10", L"0x10", 16LL, 0);
  testuns("0x10", L"0x10", 0LL, 10);
  testuns("ABC", L"ABC", 0xABCLL, 16);
  testuns("011", L"011", 3LL, 2);
  testuns("99A", L"99x", 99LL, 10);
  testuns("-99", L"-99", 0xFFFFFF9DLL, 10);
  testuns("80000000", L"80000000", 0x80000000LL, 16);
  return 0;
}
