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
#include <ctype.h>
#include <wctype.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define string__(x) #x
#define string_(x) string__(x)

static void docheck(int condition, const char *msg, int x)
{
  if (!condition) {
    fprintf(stderr, "%s failed for character 0x%02x\n", msg, x);
    abort();
  }
}

#define check(cond, x) docheck(cond, string_(cond), x)

/* test whether the macro isctype() returns true for all (and only) the
 * characters in "str"
 * this has to be a macro itself because the various isxxx functions are
 * defined both as macros and as functions
 */

#define TESTMATCH(isctype, str)        \
  do {				       \
    printf("%s ", string_(isctype));   \
    for (i = 0; i < 256; i++)	       \
      {				       \
	if (i && strchr(str, i))       \
	  check(isctype(i), i);	       \
	else			       \
	  check(!isctype(i), i);       \
      }				       \
  } while (0)



#define LOWER "abcdefghijklmnopqrstuvwxyz"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define DIGITS "0123456789"
#define BLANK " \t"
#define SPACE " \f\n\r\t\v"
#define PUNCT "~`!@#$%^&*()-_+=[]\\{}|:;\"',./<>?"

void
testmacros(void)
{
  int i;

  TESTMATCH(isalnum, LOWER UPPER DIGITS);
  TESTMATCH(isalpha, LOWER UPPER);
  TESTMATCH(isblank, BLANK);
  TESTMATCH(isdigit, DIGITS);
  TESTMATCH(isgraph, LOWER UPPER PUNCT DIGITS );
  TESTMATCH(islower, LOWER);
  TESTMATCH(isprint, LOWER UPPER PUNCT DIGITS " ");
  TESTMATCH(ispunct, PUNCT);
  TESTMATCH(isspace, SPACE);
  TESTMATCH(isupper, UPPER);
  TESTMATCH(isxdigit, "0123456789ABCDEFabcdef");
}

void
testctrlmacro(void)
{
  int i;
  printf("iscntrl ");
  for (i = 0; i < 0x20; i++) {
    check(iscntrl(i), i);
  }
  for (i = 0x20; i < 0x7f; i++) {
    check(!iscntrl(i), i);
  }
  check(iscntrl(0x7f), 0x7f);
  for (i = 0x80; i <= 0xff; i++) {
    check(!iscntrl(i), i);
  }
}

/* now check the function versions */
#undef isalnum
#undef isalpha
#undef isblank
#undef isdigit
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef islower
#undef isxdigit

void
testfuncs(void)
{
  int i;

  TESTMATCH(isalnum, LOWER UPPER DIGITS);
  TESTMATCH(isalpha, LOWER UPPER);
  TESTMATCH(isblank, BLANK);
  TESTMATCH(isdigit, DIGITS);
  TESTMATCH(isgraph, LOWER UPPER PUNCT DIGITS );
  TESTMATCH(islower, LOWER);
  TESTMATCH(isprint, LOWER UPPER PUNCT DIGITS " ");
  TESTMATCH(ispunct, PUNCT);
  TESTMATCH(isspace, SPACE);
  TESTMATCH(isupper, UPPER);
  TESTMATCH(isxdigit, "0123456789ABCDEFabcdef");
}

void
testctrlfunc(void)
{
  int i;
  printf("iscntrl ");
  for (i = 0; i < 0x20; i++) {
    check(iscntrl(i), i);
  }
  for (i = 0x20; i < 0x7f; i++) {
    check(!iscntrl(i), i);
  }
  check(iscntrl(0x7f), 0x7f);
  for (i = 0x80; i <= 0xff; i++) {
    check(!iscntrl(i), i);
  }
}

void
verifywc(unsigned char x, wchar_t wx)
{
  /* note that the iswxxx functions do not have to return
     the exact same value as isxxx, but they do have to
     be nonzero or zero at the same time
  */
  check(!!iswalnum(wx) == !!isalnum(x), wx);
  check(!!iswalpha(wx) == !!isalpha(x), wx);
  check(!!iswblank(wx) == !!isblank(x), wx);
  check(!!iswcntrl(wx) == !!iscntrl(x), wx);
  check(!!iswdigit(wx) == !!isdigit(x), wx);
  check(!!iswgraph(wx) == !!isgraph(x), wx);
  check(!!iswlower(wx) == !!islower(x), wx);
  check(!!iswprint(wx) == !!isprint(x), wx);
  check(!!iswpunct(wx) == !!ispunct(x), wx);
  check(!!iswspace(wx) == !!isspace(x), wx);
  check(!!iswupper(wx) == !!isupper(x), wx);
  check(!!iswxdigit(wx) == !!isxdigit(x), wx);

  check(iswalnum(wx) == iswctype(wx, wctype("alnum")), wx);
  check(iswalpha(wx) == iswctype(wx, wctype("alpha")), wx);
  check(iswblank(wx) == iswctype(wx, wctype("blank")), wx);
  check(iswcntrl(wx) == iswctype(wx, wctype("cntrl")), wx);
  check(iswdigit(wx) == iswctype(wx, wctype("digit")), wx);
  check(iswgraph(wx) == iswctype(wx, wctype("graph")), wx);
  check(iswlower(wx) == iswctype(wx, wctype("lower")), wx);
  check(iswprint(wx) == iswctype(wx, wctype("print")), wx);
  check(iswpunct(wx) == iswctype(wx, wctype("punct")), wx);
  check(iswspace(wx) == iswctype(wx, wctype("space")), wx);
  check(iswupper(wx) == iswctype(wx, wctype("upper")), wx);
  check(iswxdigit(wx) == iswctype(wx, wctype("xdigit")), wx);
}

int
main()
{
  int i;

  printf("testing ctype macros..."); fflush(stdout);
  testmacros();
  testctrlmacro();
  printf(" ok\n");
  printf("testing ctype functions..."); fflush(stdout);
  testfuncs();
  testctrlfunc();
  printf(" ok\n");
  printf("testing wide character ctypes..."); fflush(stdout);

  for (i = 0; i < 256; i++) {
    verifywc(i, i);
  }
  verifywc(255, 512);
  verifywc(-1, -1);
  printf(" ok\n");

  return 0;
}
