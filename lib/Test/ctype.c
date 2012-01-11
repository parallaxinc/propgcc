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

#define check(func, x) docheck(func(x), string_(func), x)

/* test whether the macro isctype() returns true for all (and only) the
 * characters in "str"
 * this has to be a macro itself because the various isxxx functions are
 * defined both as macros and as functions
 */

#define TESTMATCH(isctype, str)        \
  do {				      \
    printf("%s ", string_(isctype));  \
    for (i = 0; i < 256; i++)	      \
      {				       \
	if (i && strchr(str, i))       \
	  check(isctype, i);	       \
	else			       \
	  check(!isctype, i);	       \
      }				       \
  } while (0)


#if 0
void
verifywc(int x)
{
  assert(iswalnum(x) == isalnum(x));
  assert(iswalpha(x) == isalpha(x));
  if (x == ' ' || x == '\t')
    assert(iswblank(x));
  else
    assert(!iswblank(x));
  assert(iswcntrl(x) == iscntrl(x));
  assert(iswdigit(x) == isdigit(x));
  assert(iswgraph(x) == isgraph(x));
  assert(iswprint(x) == isprint(x));
  assert(iswpunct(x) == ispunct(x));
  assert(iswspace(x) == isspace(x));
  assert(iswxdigit(x) == isxdigit(x));
}
#endif

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
    check(iscntrl, i);
  }
  for (i = 0x20; i < 0x7f; i++) {
    check(!iscntrl, i);
  }
  check(iscntrl, 0x7f);
  for (i = 0x80; i <= 0xff; i++) {
    check(!iscntrl, i);
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
    check(iscntrl, i);
  }
  for (i = 0x20; i < 0x7f; i++) {
    check(!iscntrl, i);
  }
  check(iscntrl, 0x7f);
  for (i = 0x80; i <= 0xff; i++) {
    check(!iscntrl, i);
  }
}

int
main()
{
  printf("testing ctype macros..."); fflush(stdout);
  testmacros();
  testctrlmacro();
  printf(" ok\n");
  printf("testing ctype functions..."); fflush(stdout);
  testfuncs();
  testctrlfunc();
  printf(" ok\n");
  return 0;
}
