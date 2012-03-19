/* from Henry Spencer's stringlib */

#include <string.h>
#undef strcpy

/*
 * strcpy - copy string src to dst
 */
char *				/* dst */
strcpy(char * __restrict dst, const char * __restrict src)
{
	register char *dscan = dst;
	register const char *sscan = src;

	if (!sscan) return(dst);
	while ((*dscan++ = *sscan++) != '\0')
		continue;
	return(dst);
}
