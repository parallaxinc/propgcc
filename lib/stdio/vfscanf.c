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
#include <wchar.h>
#include <string.h>

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
#define NEXT(c) ((c)=mygetc(stream, ms_ptr))
//#define NEXT(c) ((c)=fgetc(stream),ms.size++,ms.incount++)
#define PREV(c) myungetc((c),stream, ms_ptr)
//#define PREV(c) do{if((c)!=EOF)ungetc((c),stream);ms.size--;ms.incount--;}while(0)
#define VAL(a)  ((a)&&ms_ptr->size<=width)

typedef struct mystate {
  size_t size;
  size_t incount;
  mbstate_t mbs;
} MyState;

/* get a single byte */
static int
mygetc(FILE *stream, MyState *ms)
{
  int mbc;

  mbc = fgetc(stream);
  ms->size++;
  ms->incount++;
  return mbc;
}

/* get a single character from stream; this may be
 * multiple bytes depending on locale
 * c is the first byte of the character
 * returns the last byte read, or EOF
 */
static int
mywgetc(int c, wchar_t *wc_ptr, FILE *stream, MyState *ms)
{
  size_t count;

  ms->mbs.left = 0;
  do {
    if (c < 0) {
      return c;
    }
    /* NOTE: assumes c is little-endian here */
    count = _mbrtowc_ptr(wc_ptr, (char *)&c, 1, &ms->mbs);
    if (count != ((size_t)-2)) break;
    /* need more bytes */
    c = fgetc(stream);
    ms->incount++;
  } while (1);
  if (count == (size_t)-1) {
    /* this is harsh, but makes us pass the compatiblity test */
    return EOF;
  }
  return c;
}

static void
myungetc(int c, FILE *stream, MyState *ms)
{
  ms->size--;
  ms->incount--;
  /* our ungetc will ignore EOF, no need to check here */
  ungetc(c, stream);
}

/*
 * get the next format character
 */
#define NEXTFMT(fmt) (fmt = *format_ptr++)

#ifdef WCHAR_SCANSETS
#define WNEXTFMT(fmt) (format_ptr += getFmtChar(&fmt, format_ptr, &fmts))

static int getFmtChar(wchar_t *ch, const char *fmt_ptr, mbstate_t *fmts)
{
  size_t count;
  count = _mbrtowc_ptr(ch, fmt_ptr, MB_LEN_MAX, fmts);
  if (((int)count) <= 0) {
    *ch = 0;
    return 0;
  }
  return count;
}
#endif

/*
 * see if a character is acceptable based on a scan set
 */
static int
in_scanset(int c, const char *scanset)
{
  wchar_t wc;
  wchar_t last = 0;

  if (!scanset)
    return 1;
  if (*scanset == ']') {
    if (c == ']') return 1;
    last = *scanset++;
  }
  while ( (wc = *scanset++) != 0 ) {
    if (wc == '-' && last && *scanset != ']') {
      wc = *scanset++;
      if (c >= last && c <= wc)
	return 1;
    } else if (c == wc) {
      return 1;
    }
    last = wc;
  }
  return 0;
}

/*
 * get a string from the stream
 * this may be a wide or narrow string, depending on
 * "is_wchar"
 * dest is where to put it, and width is the maximum number of characters to put
 * c is the next character from the stream
 * ignore is true if we should not assign
 * scanset is NULL if all characters are accepted, otherwise a set of characters
 * to accept or reject
 * "need_zero" is true if we should append a trailing 0
 */
static void
get_string(int is_wchar, void *dest, size_t width, int c, FILE *stream,
	   int ignore, MyState *ms_ptr,
	   const char *scanset, int need_zero)
{
  unsigned char *bp = NULL;
  wchar_t *wp = NULL;
  wchar_t wc;
  int invert; /* if 1 invert the sense of the set */

  if (scanset && *scanset == L'^') {
    invert = 1;
    scanset++;
  } else {
    invert = 0;
  }
  if (is_wchar)
    wp = dest;
  else
    bp = dest;

  while (VAL(c!=EOF))
    {
      if (!ignore)
	{
	  if (is_wchar)
	    {
	      c = mywgetc(c, &wc, stream, ms_ptr);
	      if (!(invert^in_scanset(wc, scanset)))
		break;
	      *wp++ = wc;
	    }
	  else
	    {
	      if (!(invert^in_scanset(c, scanset)))
		break;
	      *bp++ = c;
	    }
	}
      NEXT(c);
    }
  if (need_zero && !ignore && ms_ptr->size) {
    if (is_wchar)
      *wp++ = L'\0';
    else
      *bp++ = 0;
  }
  PREV(c);
}

int vfscanf(FILE *stream,const char *format_ptr,va_list args)
{
  size_t blocks=0;
  int c=0;
  int fmt=0;
  MyState ms, *ms_ptr;
#ifdef WCHAR_SCANSETS
  mbstate_t fmts;

  memset(&fmts, 0, sizeof(fmts));
#endif
  memset((ms_ptr = &ms), 0, sizeof(ms));

  NEXTFMT(fmt);
  while(fmt)
    {
      ms.size=0;

      if(fmt=='%')
	{
	  size_t width=ULONG_MAX;
	  char type,subtype='i',ignore=0;
	  const unsigned char *ptr=(const unsigned char *)(format_ptr);

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
	      ms.size=1;
	    } /* The first non-whitespace character is already read */

	  switch(type)
	    { case 'c':
		{ 
		  void *dest = NULL;

		  if(width==ULONG_MAX) /* Default */
		    width=1;
		
		  if(!ignore)
		    {
		      dest = va_arg(args, void *);
		    }

		  NEXT(c);  /* 'c' did not skip space */
		  get_string( subtype == WCHAR_SUBTYPE,
			      dest, width, c, stream, ignore, ms_ptr, NULL, 0 );
		  if(!ignore&&ms.size)
		    blocks++;
		  break;
		}
	    case '[':
	      {
		void *dest = NULL;
		const char *scanset;

		scanset = (const char *)ptr;
		if(*ptr=='^')
		    ptr++;
		if (*ptr==']')
		  ptr++;
		while (*ptr && *ptr != ']')
		  ptr++;
		if (*ptr) ptr++;

		if(!ignore)
		  dest=va_arg(args,void *);

		NEXT(c);
		get_string( subtype == WCHAR_SUBTYPE,
			    dest, width, c, stream, ignore, ms_ptr, scanset, 1 );
		if(!ignore&&ms.size)
		  {
		    blocks++;
		  }
		break;
	      }
	    case 's':
	      { void *destp = NULL;

		if(!ignore)
		  {
		    destp=va_arg(args,void *);
		  }
		get_string( subtype == WCHAR_SUBTYPE,
			    destp, width, c, stream, ignore, ms_ptr,
			    "^ \t\r\n\f\v]",
			    1 );

		if(!ignore&&ms.size)
		  { 
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
			if(ms.size==2+(min!=0)) /* No number read till now -> malformatted */
			  { PREV(c);
			    c='.';
			  }
		      }

		    if(min&&ms.size==2) /* No number read till now -> malformatted */
		      { PREV(c);
			c=min;
		      }
		    if(ms.size==1)
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

		if(!ignore&&ms.size)
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
		*va_arg(args,int *)=ms.incount;
	      ms.size=1; /* fake a valid argument */
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

		if(min&&ms.size==2) /* If there is no valid character after sign, unget last */
		  { PREV(c);
		    c=min; }

		PREV(c);

		if(ignore||!ms.size)
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
		      *va_arg(args,unsigned long *)=v;
		      break;
		    case 'i':
		      *va_arg(args,unsigned int *)=v;
		      break;
		    case 'h':
		      *va_arg(args,unsigned short *)=v;
		      break;
		    case 'H':
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
			*va_arg(args,signed long *)=v2;
		        break;
		      case 'i':
			*va_arg(args,signed int *)=v2;
			break;
		      case 'h':
			*va_arg(args,signed short *)=v2;
			break;
		      case 'H':
			*va_arg(args,signed char *)=v2;
			break;
		      }
		  }
		blocks++;
		break;
	      }
	    }
	  format_ptr=(const char *)ptr;
	  NEXTFMT(fmt);
	}else
	{ if(isspace(fmt))
	    { do
		NEXT(c);
	      while(isspace(c));
	      PREV(c);
	      ms.size=1; }
	  else
	    { NEXT(c);
	      if(c!=fmt)
		PREV(c); }
	  NEXTFMT(fmt);
	}
      if(!ms.size)
	break;
    }

  if(c==EOF&&!blocks)
    return c;
  else
    return blocks;
}
