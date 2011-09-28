/* from Henry Spencer's stringlib */

#include <string.h>

/*
 * strncpy - copy at most n characters of string src to dst
 */
char *				/* dst */
strncpy(char *dst, const char *src, size_t n)
{
	register char *dscan;
	register const char *sscan;
	register long count;

	dscan = dst;
	sscan = src;
	count = n;
	while (--count >= 0 && (*dscan++ = *sscan++) != '\0')
		continue;
	while (--count >= 0)
		*dscan++ = '\0';
	return(dst);
}
