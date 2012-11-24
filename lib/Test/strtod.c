/*
 * Tests for strtod function
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
#include <stdint.h>

#define string__(x) #x
#define string_(x) string__(x)

int fails = 0;

static int checkd(double d, double expect)
{
  union d_or_i {
    double d;
    int64_t i;
  } DX, DE;

  /* check on the bit representation, because "NaN == NaN" will
     always fail! */
  DX.d = d;
  DE.d = expect;
  if (DX.i != DE.i) {
    int64_t delta;
    delta = DX.i - DE.i;
    if (delta == 1 || delta == -1) {
      fprintf(stderr, "warning: expected 0x%llx got 0x%llx (1 ULP error)\n", DE.i, DX.i);
    } else {
      fprintf(stderr, "check failed: expected 0x%llx, got 0x%llx\n", DE.i, DX.i);
      fails++;
    }
    return 0;
  }
  return 1;
}

static int checkf(float d, float expect)
{
  union f_or_i {
    float f;
    int32_t i;
  } DX, DE;

  DX.f = d;
  DE.f = expect;
  if (DX.i != DE.i) {
    int32_t delta;
    delta = DX.i - DE.i;
    if (delta == 1 || delta == -1) {
      fprintf(stderr, "warning: expected 0x%x got 0x%x (1 ULP error)\n", DE.i, DX.i);
    } else {
      fprintf(stderr, "check failed: expected 0x%x, got 0x%x\n", DE.i, DX.i);
      fails++;
    }
    return 0;
  }
  return 1;
}


void
testdouble(const char *str, double expect, float expectf, char nextc)
{
  double xl;
  float xf;
  int ok;
  char *ptr;

  printf("testing [%s] strtod ", str); fflush(stdout);
  xl = strtod(str, &ptr);
  ok = checkd(xl, expect);
  if (*ptr != nextc) {
    fprintf(stderr, "got wrong character at end of string (expected %c, got %c)\n", nextc, *ptr);
    ok = 0;
  }
  printf("strtof "); fflush(stdout);
  xf = strtof(str, &ptr);
  ok = checkf(xf, expectf) && ok;
  if (*ptr != nextc) {
    fprintf(stderr, "got wrong character at end of string (expected %c, got %c)\n", nextc, *ptr);
    ok = 0;
  }
  printf("%s\n", ok ? "ok" : "");

}

int
main()
{
  testdouble("0ABC", 0, 0.0f, 'A');
  testdouble("1ABC", 1, 1.0f, 'A');
  testdouble("2.0XYZ", 2, 2.0f, 'X');
  testdouble("1.23e2+4", 123.0, 123.0f, '+');
  testdouble("-12.0DEF", -12.0, -12.0f, 'D');
  testdouble("0.1.-", 0.1, 0.1f, '.');
  testdouble("3.14159fg", 3.14159, 3.14159f, 'f');
  testdouble("0.81mno", 0.81, 0.81f, 'm');
  testdouble("999.999zz", 999.999, 999.999f, 'z');
  testdouble("12345678901234.5ABC", 12345678901234.5, 12345678901234.5f, 'A');
  testdouble("-7.456e-125AB", -7.456e-125, -0.0f, 'A');
  testdouble("9.87e+64XY", 9.87e+64, __builtin_inff(), 'X');
  testdouble("0x1.0P0-+", 1.0, 1.0f, '-');
  testdouble("0x1.9P8AX", 0x1.9P8, 0x1.9P8f, 'A');
  testdouble("0x1.Ap8AB", 0x1.AP8, 0x1.AP8f, 'A');
  testdouble("0x1.abcdefp-55ABC", 0x1.abcdefP-55, 0x1.abcdefp-55f, 'A');
  testdouble("144115188075855877, 2", 144115188075855877.0, 144115188075855877.0f, ',');
  testdouble("infab", __builtin_inf(), __builtin_inff(), 'a');
  testdouble("nan(1)ZY", __builtin_nan("1"), __builtin_nanf("1"), 'Z');

  return fails;
}
