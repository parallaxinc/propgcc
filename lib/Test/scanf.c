/*
 * tests for scanf
 */

#define _GNU_SOURCE /* allow fmemopen function */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <assert.h>

#define string__(x) #x
#define string_(x) string__(x)

int numerrors = 0;

static void docheck_str(const char *buf, const char *expect, int line)
{
  if (strcmp(buf, expect) != 0) {
    fprintf(stderr, "test failed on line %d: got [%s] expected [%s]\n", line, buf, expect);
    numerrors++;
  }
}

static void docheck_wstr(const wchar_t *buf, const wchar_t *expect, int line)
{
  if (wcscmp(buf, expect) != 0) {
    fprintf(stderr, "test failed on line %d: got {%ls} expected {%ls}\n", line, buf, expect);
    numerrors++;
  }
}

static void checkequal(const char *msg, int num, int expect, int line)
{
  if (num != expect) {
    fprintf(stderr, "test %s failed on line %d: got %d expected %d\n", msg, line, num, expect);
    numerrors++;
  }
}
#define EXPECTEQ(var, expect) checkequal(string_(var), var, expect, __LINE__)
#define checkstr(buf, expect) docheck_str(buf, expect, __LINE__)
#define checkwstr(buf, expect) docheck_wstr(buf, expect, __LINE__)

#define teststr(inp, fmt, expect) \
  {				  \
    buf[0] = 0;			  \
    sscanf(inp, fmt, &buf[0]);	  \
    checkstr(buf, expect);	  \
  }
#define test2str(inp, fmt, expect1, expect2)		\
  {				  \
    buf[0] = 0;	buf2[0] = 0;	  \
    sscanf(inp, fmt, &buf[0], &buf2[0]);	  \
    checkstr(buf, expect1);	  \
    checkstr(buf2, expect2);	  \
  }
#define testwstr(inp, fmt, expect) \
  {				  \
    memset(wbuf, 0, sizeof(wbuf));	\
    sscanf(inp, fmt, &wbuf[0]);	  \
    checkwstr(wbuf, expect);	  \
  }
#define test2wstr(inp, fmt, expect1, expect2)	\
  {						\
    memset(wbuf, 0, sizeof(wbuf));		\
    memset(wbuf2, 0, sizeof(wbuf2));		\
    sscanf(inp, fmt, &wbuf[0], &wbuf2[0]);	\
    checkwstr(wbuf, expect1);			\
    checkwstr(wbuf2, expect2);			\
  }

char buf[128];
char buf2[128];
wchar_t wbuf[128];
wchar_t wbuf2[128];

/* start of a prefix */
char test0x[] = "0xyz";

int
main()
{
  int count;
  int tmp;
  int nextc;
  FILE *f;

  printf("checking: chars "); fflush(stdout);
  teststr("abcd", "%c", "a");
  teststr(" abcd", "%2c", " a");

  printf("strings ");
  teststr("abcd", "%3s", "abc");
  teststr("a", "%1s", "a");
  teststr("  abcd", "%5s", "abcd");
  teststr("hello world", "%s", "hello");
  teststr("THE0bestA", "%[A-Z]", "THE");
  teststr("1THE0bestA", "%[0-9a-z]", "1");
  teststr("not here", "%[0-9]", "");
  teststr("0ab9AB7", "%[0-9A-Z]", "0");
  teststr("7THE0bestA", "%[a-z0-9]", "7");
  teststr("123456789", "%3[0-9]", "123");
  teststr("ab123cd", "%4[^0-9]", "ab");
  teststr("[[a", "%[[]", "[[");
  teststr("ab]]", "%[^]]", "ab");
  teststr("ab]]", "%[]]", "");
  teststr("]]cd", "%[]]", "]]");

  setlocale(LC_ALL, "");

  printf("wchars "); fflush(stdout);
#define AlphaBetaGamma "\u03B1\u03B2\u03B3"

  testwstr(AlphaBetaGamma, "%lc", L"\u03B1");

  printf("wstrings "); fflush(stdout);
  testwstr(" abcd", "%3ls", L"abc");
  testwstr(" \u03B1  ", "%ls", L"\u03B1");
  testwstr(AlphaBetaGamma, "%ls", L"\u03B1\u03B2\u03B3");
  testwstr(AlphaBetaGamma, "%1ls", L"\u03B1");
  testwstr(AlphaBetaGamma, "%2ls", L"\u03B1\u03B2");
  testwstr(AlphaBetaGamma, "%3ls", L"\u03B1\u03B2\u03B3");
  testwstr("ab123cd", "%4l[^0-9]", L"ab");
  testwstr("[[a", "%l[[]", L"[[");
  testwstr("ab]]", "%l[^]]", L"ab");
  testwstr("ab]]", "%l[]]", L"");
  testwstr("]]cd", "%l[]]", L"]]");
  testwstr(AlphaBetaGamma, "%l[\u03B1\u03B2]", L"\u03B1\u03B2");
  testwstr(AlphaBetaGamma, "%l[^\u03B2]", L"");

  memset(wbuf, 0, sizeof(wbuf));
  testwstr("a\x80R", "%ls", L"a");

  printf("done\n");

  printf("testing 2 strings: "); fflush(stdout);
  test2str("12345 ab", "%s %s", "12345", "ab");
  test2str("12345ab", "%[0-9]%s", "12345", "ab");
  test2str("hello bob, how are you today?", "hello %[^,], how are %s today?", "bob", "you");

  test2wstr("12345\u0310ab", "%l[0-9]%ls", L"12345", L"\u0310ab");
  printf("done\n");

  printf("checking some corner cases... "); fflush(stdout);
  f = fmemopen(test0x, strlen(test0x), "r");
  tmp = -1;
  count = fscanf(f, "%d", &tmp);
  nextc = fgetc(f);
  EXPECTEQ(count, 1);
  EXPECTEQ(tmp, 0);
  EXPECTEQ(nextc, 'x');
  rewind(f);
  count = fscanf(f, "%c", &buf[0]);
  nextc = fgetc(f);
  EXPECTEQ(count, 1);
  EXPECTEQ(buf[0], '0');
  EXPECTEQ(nextc, 'x');

  rewind(f);
  tmp = 12;
  count = fscanf(f, "0%x", &tmp);
  nextc = fgetc(f);
  EXPECTEQ(count, 0);
  EXPECTEQ(tmp, 12);
  EXPECTEQ(nextc, 'x');

  /* this test is a little bit nasty; the string "0xyz" has a valid
     hex prefix, but is not a valid number. strtol will return 0 for it
     and leave the pointer at "x", but scanf can't because the
     standard doesn't allow more than 1 character of pushback;
     so it has to fail and leave the pointer at "y". GLIBC on Linux actually
     gets this wrong :-(
  */
  rewind(f);
  count = fscanf(f, "%x", &tmp);
  nextc = fgetc(f);
  EXPECTEQ(count, 0);
  EXPECTEQ(nextc, 'y');
  rewind(f);
  count = fscanf(f, "%i", &tmp);
  nextc = fgetc(f);
  EXPECTEQ(count, 0);
  EXPECTEQ(nextc, 'y');

  fclose(f);

  count = sscanf("0178ag", "%i%c", &tmp, &buf[0]);
  EXPECTEQ(count, 2);
  EXPECTEQ(tmp, 017);
  EXPECTEQ(buf[0], '8');
  count = sscanf("0178ag", "%d%c", &tmp, &buf[0]);
  EXPECTEQ(count, 2);
  EXPECTEQ(tmp, 178);
  EXPECTEQ(buf[0], 'a');
  count = sscanf("0178ag", "%x%c", &tmp, &buf[0]);
  EXPECTEQ(count, 2);
  EXPECTEQ(tmp, 0x178a);
  EXPECTEQ(buf[0], 'g');
  count = sscanf("0178ag", "%o%c", &tmp, &buf[0]);
  EXPECTEQ(count, 2);
  EXPECTEQ(tmp, 15);
  EXPECTEQ(buf[0], '8');

  strcpy(buf, "xxxxxx");
  strcpy(buf2, "xxxxxx");
  wcscpy(wbuf, L"xxxxxx");
  count = sscanf("ab\u0311\u0312cd", "a%[a-z]%l[^a-z]%c", buf, wbuf, buf2);
  EXPECTEQ(count, 3);
  EXPECTEQ(strcmp(buf, "b"), 0);
  EXPECTEQ(wbuf[0], L'\u0311');
  EXPECTEQ(wbuf[1], L'\u0312');
  EXPECTEQ(strcmp(buf2, "cxxxxx"), 0);

  printf("done\n");

  if (numerrors > 0) {
    printf("*** TEST FAILURES! ***\n");
  }
  return numerrors;
}
