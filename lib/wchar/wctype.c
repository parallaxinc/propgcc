#include <wctype.h>
#include <string.h>

static struct wcmap {
  const char *str;
  wctype_t ctype;
} wcmap[] = {
  { "alnum", _CTalnum },
  { "alpha", _CTalpha },
  { "blank", _CTb },
  { "cntrl", _CTc },
  { "digit", _CTd },
  { "graph", _CTgraph },
  { "lower", _CTl },
  { "print", _CTprint },
  { "punct", _CTp },
  { "space", _CTs },
  { "upper", _CTu },
  { "xdigit", _CTx },
};

#define NUM_WCTYPES (sizeof(wcmap)/sizeof(wcmap[0]))

wctype_t
wctype(const char *property)
{
  int i;

  for (i = 0; i < NUM_WCTYPES; i++) {
    if (!strcmp(wcmap[i].str, property))
      return wcmap[i].ctype;
  }
  return 0;
}

