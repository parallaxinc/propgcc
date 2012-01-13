/*
 * originally from the libnix library for the Amiga, written by
 * Matthias Fleischer and Gunther Nikl and placed in the public domain
 *
 * Propeller specific changes by Eric R. Smith,
 * Copyright (c)2011 Parallax Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <math.h>
#include <compiler.h>

#define MAX(x,y) ((x)>(y)?(x):(y))
#define MIN(x,y) ((x)<(y)?(x):(y))

#define WCHAR_SUBTYPE 'l'

/*
 * define FLOAT_SUPPORT to get floating point support
 */
/*#define FLOAT_SUPPORT*/

/* some macros to cut this short
 * NEXT(c);     read next character
 * PREV(c);     ungetc a character
 * VAL(a)       leads to 1 if a is true and valid
 */
#define NEXT(c) ((c)=fgetc(stream),size++,incount++)
#define PREV(c) do{if((c)!=EOF)ungetc((c),stream);size--;incount--;}while(0)
#define VAL(a)  ((a)&&size<=width)


int vfscanf(FILE *stream,const char *format,va_list args)
{
  size_t blocks=0,incount=0;
  int c=0;

  while(*format)
    {
      size_t size=0;

      if(*format=='%')
	{
	  size_t width=ULONG_MAX;
	  char type,subtype='i',ignore=0;
	  const unsigned char *ptr=(const unsigned char *)(format+1);
	  size_t i;

	  if(isdigit(*ptr))
	    {
	      width=0;
	      while(isdigit(*ptr))
		width=width*10+(*ptr++-'0');
	    }

	  if (*ptr=='*')
	    {
	      ptr++;
	      ignore=1;
	    }

	  if(*ptr=='h'||*ptr=='l'||*ptr=='L'||*ptr=='j'||*ptr=='t'||*ptr=='z')
	    {
	      subtype = *ptr++;
	      if (*ptr == subtype)
		{
		  subtype=toupper(subtype);
		  ptr++;
		}
	    }

	  type=*ptr++;

	  if(type&&type!='%'&&type!='c'&&type!='n'&&type!='[')
	    { do /* ignore leading whitespace characters */
		NEXT(c);
	      while(isspace(c));
	      size=1;
	    } /* The first non-whitespace character is already read */

	  switch(type)
	    { case 'c':
		{ unsigned char *bp = NULL;
		wchar_t *wp = NULL;

		if(width==ULONG_MAX) /* Default */
		  width=1;
		
		if(!ignore)
		  {
		    if (subtype == WCHAR_SUBTYPE)
		      wp = va_arg(args, wchar_t *);
		    else
		      bp = va_arg(args, unsigned char *);
		  }

		NEXT(c); /* 'c' did not skip whitespace */
		while(VAL(c!=EOF))
		  { if(!ignore)
		      {
			if (subtype == WCHAR_SUBTYPE)
			  *wp++=c;
			else
			  *bp++=c;
		      }
		    NEXT(c);
		  }
		PREV(c);

		if(!ignore&&size)
		  blocks++;
		break;
		}
	    case '[':
	      {
		unsigned char *bp;
		unsigned char tab[32],a,b;
		char circflag=0;

		if(*ptr=='^')
		  {
		    circflag=1;
		    ptr++;
		  }
		for(i=0;i<sizeof(tab);i++)
		  tab[i]=circflag?255:0;
		for(;;)
		  { if(!*ptr)
		      break;
		    a=b=*ptr++;
		    if(*ptr=='-'&&ptr[1]&&ptr[1]!=']')
		      { ptr++;
			b=*ptr++; }
		    for(i=a;i<=b;i++)
		      if(circflag)
			tab[i/8]&=~(1<<(i&7));
		      else
			tab[i/8]|=1<<(i&7);
		    if(*ptr==']')
		      { ptr++;
			break; }
		  }

		if(!ignore)
		  bp=va_arg(args,unsigned char *);
		else
		  bp=NULL; /* Just to get the compiler happy */

		NEXT(c);
		while(VAL(c!=EOF&&tab[c/8]&(1<<(c&7))))
		  { if(!ignore)
		      *bp++=c;
		    NEXT(c);
		  }
		PREV(c);

		if(!ignore&&size)
		  { *bp++='\0';
		    blocks++; }
		break;
	      }
	    case 's':
	      { unsigned char *bp=NULL;
		wchar_t *wp=NULL;

		if(!ignore)
		  {
		    if (subtype==WCHAR_SUBTYPE)
		      wp=va_arg(args,wchar_t *);
		    else
		      bp=va_arg(args,unsigned char *);
		  }

		while(VAL(c!=EOF&&!isspace(c)))
		  { if(!ignore)
		      {
			if (subtype==WCHAR_SUBTYPE)
			  *wp++=c;
			else
			  *bp++=c;
		      }
		    NEXT(c);
		  }
		PREV(c);

		if(!ignore&&size)
		  { 
		    if (subtype==WCHAR_SUBTYPE)
		      *wp++=L'\0';
		    else
		      *bp++='\0';
		    blocks++;
		  }
		break;
	      }
#ifdef FLOAT_SUPPORT
	    case 'e':
	    case 'f':
	    case 'g':
	      { long double v;
		int ex=0;
		int min=0,mine=0; /* This is a workaround for gcc 2.3.3: should be char */
		
		do /* This is there just to be able to break out */
		  {
		    if(VAL(c=='-'||c=='+'))
		      { min=c;
			NEXT(c); }

		    if(VAL(tolower(c)=='i')) /* +- inf */
		      { int d;
			NEXT(d);
			if(VAL(tolower(d)=='n'))
			  { int e;
			    NEXT(e);
			    if(VAL(tolower(e)=='f'))
			      { v=HUGE_VAL;
				if (min=='-') v=-v;
				break; } /* break out */
			    PREV(e);
			  }
			PREV(d);
		      }
		    else if(VAL(toupper(c)=='N')) /* NaN */
		      { int d;
			NEXT(d);
			if(VAL(tolower(d)=='a'))
			  { int e;
			    NEXT(e);
			    if(VAL(toupper(e)=='N'))
			      { v=_NAN;
				break; }
			    PREV(e);
			  }
			PREV(d);
		      }

		    v=0.0;
		    while(VAL(isdigit(c)))
		      { v=v*10.0+(c-'0');
			NEXT(c);
		      }

		    if(VAL(c=='.'))
		      { long double dp=0.1;
			NEXT(c);
			while(VAL(isdigit(c)))
			  { v=v+dp*(c-'0');
			    dp=dp/10.0;
			    NEXT(c); }
			if(size==2+(min!=0)) /* No number read till now -> malformatted */
			  { PREV(c);
			    c='.';
			  }
		      }

		    if(min&&size==2) /* No number read till now -> malformatted */
		      { PREV(c);
			c=min;
		      }
		    if(size==1)
		      break;

		    if(VAL(tolower(c)=='e'))
		      { int d;
			NEXT(d);
			if(VAL(d=='-'||d=='+'))
			  { mine=d;
			    NEXT(d);
			  }

			if(VAL(isdigit(d)))
			  { do
			      { ex=ex*10+(d-'0');
				NEXT(d);
			      }while(VAL(isdigit(d)&&ex<100));
			    c=d;
			  } else
			  { PREV(d);
			    if(mine)
			      PREV(mine);
			  }
		      }
		    PREV(c);

		    if(mine=='-')
		      v=v/pow(10.0,ex);
		    else
		      v=v*pow(10.0,ex);

		    if(min=='-')
		      v=-v;

		  }while(0);

		if(!ignore&&size)
		  { switch(subtype)
		      {
		      case 'L':
			*va_arg(args,long double*)=v;
			break;
		      case 'l':
			*va_arg(args,double *)=v;
		      break;
		      case 'i':
		      default:
			*va_arg(args,float *)=v;
			break;
		      }
		    blocks++;
		  }
		break;
	      }
#endif
	    case '%':
	      NEXT(c);
	      if(c!='%')
		PREV(c); /* unget non-'%' character */
	      break;
	    case 'n':
	      if(!ignore)
		*va_arg(args,int *)=incount;
	      size=1; /* fake a valid argument */
	      blocks++;
	      break;
	    default:
	      { unsigned long long v=0;
		int base;
		int min=0;

		if(!type)
		  ptr--; /* unparse NUL character */

		if(type=='p')
		  { subtype='l'; /* This is the same as %lx */
		    type='x'; }

		if(VAL((c=='-'&&type!='u')||c=='+'))
		  { min=c;
		    NEXT(c); }

		if(type=='i') /* which one to use ? */
		  { if(VAL(c=='0')) /* Could be octal or sedecimal */
		      { int d;
			NEXT(d); /* Get a look at next character */
			if(VAL(tolower(d)=='x'))
			  { int e;
			    NEXT(e); /* And the next */
			    if(VAL(isxdigit(c)))
			      type='x'; /* Is a valid x number with '0x?' */
			    PREV(e);
			  }else
			  type='o';
			PREV(d);
		      }else if(VAL(!isdigit(c)&&isxdigit(c)))
		      type='x'; /* Is a valid x number without '0x' */
		  }
		
		while(type=='x'&&VAL(c=='0')) /* sedecimal */
		  { int d;
		    NEXT(d);
		    if(VAL(tolower(d)=='x'))
		      { int e;
			NEXT(e);
			if(VAL(isxdigit(e)))
			  { c=e;
			    break; } /* Used while just to do this ;-) */
			PREV(e);
		      }
		    PREV(d);
		    break; /* Need no loop */
		  }

		base=type=='x'||type=='X'?16:(type=='o'?8:10);
		while(VAL(isxdigit(c)&&(base!=10||isdigit(c))&&(base!=8||c<='7')))
		  { v=v*base+(isdigit(c)?c-'0':0)+(isupper(c)?c-'A'+10:0)+(islower(c)?c-'a'+10:0);
		    NEXT(c);
		  }

		if(min&&size==2) /* If there is no valid character after sign, unget last */
		  { PREV(c);
		    c=min; }

		PREV(c);

		if(ignore||!size)
		  break;

		if(type=='u')
		  switch(subtype)
		    {
		    case 'L':
		      *va_arg(args,unsigned long long *)=v;
		      break;
		    case 'l':
		    case 'z':
		    case 'j':
		    case 't':
		      v = MIN(v, ULONG_MAX);
		      *va_arg(args,unsigned long *)=v;
		      break;
		    case 'i':
		      v = MIN(v, UINT_MAX);
		      *va_arg(args,unsigned int *)=v;
		      break;
		    case 'h':
		      v = MIN(v, USHRT_MAX);
		      *va_arg(args,unsigned short *)=v;
		      break;
		    case 'H':
		      v = MIN(v, UCHAR_MAX);
		      *va_arg(args,unsigned char *)=v;
		      break;
		    }
		else
		  { signed long long v2;
		    if(min=='-')
		      v2=-v;
		    else
		      v2=v;
		    switch(subtype)
		      {
		      case 'L':
			*va_arg(args,signed long long *)=v2;
		        break;
		      case 'l':
		      case 'z':
		      case 'j':
		      case 't':
			v = MAX(MIN(v, LONG_MAX), LONG_MIN);
			*va_arg(args,signed long *)=v2;
		        break;
		      case 'i':
			v = MAX(MIN(v, INT_MAX), INT_MIN);
			*va_arg(args,signed int *)=v2;
			break;
		      case 'h':
			v = MAX(MIN(v, SHRT_MAX), SHRT_MIN);
			*va_arg(args,signed short *)=v2;
			break;
		      case 'H':
			v = MAX(MIN(v, SCHAR_MAX), SCHAR_MIN);
			*va_arg(args,signed char *)=v2;
			break;
		      }
		  }
		blocks++;
		break;
	      }
	    }
	  format=(const char *)ptr;
	}else
	{ if(isspace(*format))
	    { do
		NEXT(c);
	      while(isspace(c));
	      PREV(c);
	      size=1; }
	  else
	    { NEXT(c);
	      if(c!=*format)
		PREV(c); }
	  format++;
	}
      if(!size)
	break;
    }

  if(c==EOF&&!blocks)
    return c;
  else
    return blocks;
}
