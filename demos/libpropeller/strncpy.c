/* from Henry Spencer's stringlib */
/* check for NULL string added by ERS */

#include <string.h>

/*
 * strncpy - copy at most n characters of string src to dst
 */
char *				/* dst */
strncpy(dst, src, n)
char *dst;
const char *src;
size_t n;
{
	register char *dscan;
	register const char *sscan;
	register long count;

	dscan = dst;
	if ((sscan = src) == NULL)
		sscan = "";
	count = n;
	while (--count >= 0 && (*dscan++ = *sscan++) != '\0')
		continue;
	while (--count >= 0)
		*dscan++ = '\0';
	return(dst);
}
