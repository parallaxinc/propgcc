/*
 * from the libnix library for the Amiga, written by
 * Matthias Fleischer and Gunther Nikl and placed in the public
 * domain
 */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <compiler.h>

signed long long strtoll(const char *nptr,char **endptr,int base)
{ const char *p=nptr;
  char *q;
  unsigned long long r;
  while(isspace(*p))
    p++;
  r=strtoul(p,&q,base);
  if(endptr!=NULL)
  { if(q==p)
      *endptr=(char *)nptr;
    else
      *endptr=q;
  }
  if(*p=='-')
  { if((signed long)r>0)
    { errno=ERANGE;
      return LLONG_MIN; }
    else
      return r;
  }else
  { if((signed long)r<0)
    { errno=ERANGE;
      return LLONG_MAX; }
    else
      return r;
  }
}

__weak_alias(strtoimax, strtoll);
