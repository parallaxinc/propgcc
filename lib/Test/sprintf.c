/*
 * test the printf family of functions
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>
#include <locale.h>

#define string__(x) #x
#define string_(x) string__(x)

static void docheck(const char *buf, const char *expect, int line)
{
  if (strcmp(buf, expect) != 0) {
    fprintf(stderr, "test failed on line %d: got [%s] expected [%s]\n", line, buf, expect);
    abort();
  }
}

#define check(buf, expect) docheck(buf, expect, __LINE__)

#define TEST1(fmt, arg1, compare)		\
  do {						\
    sprintf(buf, fmt, arg1);			\
    check(buf, compare);		\
  } while (0);					\


char buf[128];

int
main()
{
  printf("sprintf tests: characters "); fflush(stdout);
  TEST1("%c", 'Z', "Z");
  TEST1("%2c", 'a', " a");
  TEST1("%lc", L'#', "#");

  printf("strings "); fflush(stdout);
  TEST1("%s", "hello", "hello");
  TEST1("%.2s", "hello", "he");
  TEST1("%.2s", "", "");
  TEST1("%ls", L"this is a test", "this is a test");

  printf("ints "); fflush(stdout);
  TEST1("%02d", 4, "04");
  TEST1("%04x", 16, "0010");
  TEST1("%04X", 10, "000A");
  printf("ok\n");

  printf("testing wide characters: "); fflush(stdout);
  // \u00eb is lower case e with umlaut
  // \u00f8 is lower case o with slash
  TEST1("%ls", L"h\u00ebll\u00f8", "h?ll?");
  printf("ok\n");

  printf("now testing UTF-8: "); fflush(stdout);
  setlocale(LC_ALL, "");
  TEST1("%ls", L"h\u00ebll\u00f8", "h\xc3\xabll\xc3\xb8");

  return 0;
}
