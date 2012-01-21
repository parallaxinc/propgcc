/*
 * tests for scanf
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#define string__(x) #x
#define string_(x) string__(x)

static void docheck_str(const char *buf, const char *expect, int line)
{
  if (strcmp(buf, expect) != 0) {
    fprintf(stderr, "test failed on line %d: got [%s] expected [%s]\n", line, buf, expect);
    abort();
  }
}

static void docheck_wstr(const wchar_t *buf, const wchar_t *expect, int line)
{
  if (wcscmp(buf, expect) != 0) {
    fprintf(stderr, "test failed on line %d: got {%ls} expected {%ls}\n", line, buf, expect);
    abort();
  }
}

#define checkstr(buf, expect) docheck_str(buf, expect, __LINE__)
#define checkwstr(buf, expect) docheck_wstr(buf, expect, __LINE__)

#define teststr(inp, fmt, expect) \
  {				  \
    buf[0] = 0;			  \
    sscanf(inp, fmt, &buf[0]);	  \
    checkstr(buf, expect);	  \
  }
#define testwstr(inp, fmt, expect) \
  {				  \
    memset(wbuf, 0, sizeof(wbuf));	\
    sscanf(inp, fmt, &wbuf[0]);	  \
    checkwstr(wbuf, expect);	  \
  }

char buf[128];
wchar_t wbuf[128];

int
main()
{
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

  memset(wbuf, 0, sizeof(wbuf));
  testwstr("a\x80R", "%ls", L"a");


  printf("ok\n");
  return 0;
}
