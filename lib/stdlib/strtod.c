/*
 * @strtod.c
 * Convert string to double.
 *
 * Copyright (c) 2011 Parallax, Inc.
 * Written by Eric R. Smith, Total Spectrum Software Inc.
 * MIT licensed (see terms at end of file)
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <compiler.h>

/*
 * calculate v * 10^n
 *
 * e.g. for v * 10^13 (where 13 = 0xD == 0b01101) 
 *
 * calc = 10.0 * 10^4 * 10^8
 */
static double
__mul_pow_ten(double v, int n)
{
  double powten = 10.0;
  double calc = 1.0;
  int minus = 0;

  if (n < 0)
    {
      minus = 1;
      n = -n;
    }
  while (n > 0)
    {
      if (n & 1)
	{
	    calc = calc * powten;
	}
      powten = powten * powten;
      n = n>>1;
    }
  if (minus)
    v = v/calc;
  else
    v = v*calc;
  return v;
}

double
strtod(const char *str, char **endptr)
{
  double v = 0.0;
  int minus = 0;
  int minuse = 0;
  int c;
  int exp = 0;

  while (isspace(*str))
    str++;
  if (*str == '-')
    {
      minus = 1;
      str++;
    }
  else if (*str == '+')
    str++;

  c = toupper(*str++);
  if (c == 'I') {
    if (toupper(str[0]) == 'N' && toupper(str[1]) == 'F')
      {
	str += 2;
	v = HUGE_VAL;
	if (minus) v = -v;
	goto done;
      }
  } else if (c == 'N') {
    if (toupper(str[0]) == 'A' && toupper(str[1]) == 'N')
      {
	str += 2;
	if (*str == '(')
	  {
	    /* we actually should parse this, but it's all implementation
	       defined anyway */
	    do {
	      c = *str++;
	    } while (c != ')');
	  }
	v = _NAN;
	if (minus) v = -v;
	goto done;
      }
  }

  while (isdigit(c)) {
    v = 10.0 * v + (c - '0');
    c = *str++;
  }
  if (c == '.') {
    double frac = 0.1;
    c = *str++;
    while (isdigit(c))
      {
	v = v + frac*(c-'0');
	frac = frac * 0.1;
	c = *str++;
      }
  }
  if (c == 'e' || c == 'E') 
    {
      c = *str++;
      if (c == '-') {
	minuse = 1;
	c = *str++;
      } else if (c == '+') {
	c = *str++;
      }
      while (isdigit(c))
	{
	  exp = 10*exp + (c-'0');
	  c = *str++;
	}
      if (minuse)
	{
	  exp = -exp;
	}
      v = __mul_pow_ten(v, exp);
    }

 done:
  if (endptr)
    *endptr = (char *)str;
  if (minus)
    v = -v;
  return v;
}

/* +--------------------------------------------------------------------
 * ¦  TERMS OF USE: MIT License
 * +--------------------------------------------------------------------
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
 * +--------------------------------------------------------------------
 */
