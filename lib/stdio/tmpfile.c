#include <stdio.h>
#include <stdlib.h>
#include <propeller.h>

struct tmpf {
  struct tmpf *next;
  char namebuf[L_tmpnam];
};

static HUBDATA struct tmpf *deletelist;
static HUBDATA int delete_registered;
static HUBDATA _atomic_t dellock;

static void
deletetemps()
{
  struct tmpf *s, *nexts;
  __lock(&dellock);
  s = deletelist;
  while (s) {
    remove(s->namebuf);
    nexts = s->next;
    free(s);
    s = nexts;
  }
  __unlock(&dellock);
}

FILE *tmpfile(void)
{
  FILE *f = NULL;
  char *name;
  struct tmpf *del;

  del = malloc(sizeof(*del));
  name = tmpnam(del->namebuf);
  if (name) {
    f = fopen(name, "w+");
    if (f) {
      __lock(&dellock);
      del->next = deletelist;
      deletelist = del;
      if (!delete_registered) {
	atexit(deletetemps);
	delete_registered = 1;
      }
      __unlock(&dellock);
    }
  }
  return f;
}
