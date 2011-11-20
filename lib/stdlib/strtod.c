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
#include <errno.h>

/*
 * calculate v * base^n
 *
 * e.g. for v * 10^13 (where 13 = 0xD == 0b01101) 
 *
 * calc = 10.0 * 10^4 * 10^8
 */
static long double
__mul_pow(long double v, int n, long double base)
{
  long double powten = base;
  long double calc = 1.0;
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

long double
strtold(const char *str, char **endptr)
{
  long double v = 0.0;
  long double base = 10.0;
  long double invbase = 0.1;
  int hex = 0;
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
	v = HUGE_VALL;
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
	v = _NANL;
	if (minus) v = -v;
	goto done;
      }
  }

  if (c == '0' && toupper(*str) == 'X') {
    hex = 1;
    base = 16.0;
    invbase = (1.0/16.0);
    str++;
    c = toupper(*str++);
  }

  while (isdigit(c) || (hex && isxdigit(c) && (c -= ('A' - '0')))) {
    v = base * v + (c - '0');
    c = toupper(*str++);
  }
  if (c == '.') {
    long double frac = invbase;
    c = toupper(*str++);
    while (isdigit(c) || (hex && isxdigit(c) && (c -= ('A' - '0'))))
      {
	v = v + frac*(c-'0');
	frac = frac * invbase;
	c = *str++;
      }
  }
  if (c == 'E' || c == 'P') 
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
      v = __mul_pow(v, exp, hex ? 2.0 : 10.0);
    }

  if (v == HUGE_VALL)
    errno = ERANGE;

 done:
  if (endptr)
    *endptr = (char *)str;
  if (minus)
    v = -v;
  return v;
}

#if defined(__PROPELLER_64BIT_DOUBLES__)
double strtod(const char *str, char **endptr) __attribute__((alias("strtold")));
#endif

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
